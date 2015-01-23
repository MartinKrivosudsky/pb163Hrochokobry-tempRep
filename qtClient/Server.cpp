#include <iostream>
#include "Server.h"
#include "Client.h"
#include <time.h>
#include <unistd.h>
#include "tableone.h"

#include "polarssl/entropy.h"
#include "polarssl/ctr_drbg.h"
#include "polarSSL/sha256.h"

Server::Server( int port, QObject* parent )
:   QTcpServer( parent ),
    client( NULL ),
    m_key (false),
    counter(0),
    m_crypted(true),
    prepare1(true),
    prepare2(false),
    using_table(true),
    helpbool(true)
{
    tableOne = new prepare_table;
    tableOne->p_table = NULL;
    tableOne->table_length = 20480;
    tableOne->counter = 0;

    tableTwo = new prepare_table;
    tableTwo->p_table = NULL;
    tableTwo->table_length = 20480;
    tableTwo->counter = 0;

    TableOne * toc = new TableOne(this);
    connect(this, SIGNAL(tableOneSignal(prepare_table*)), toc, SLOT (TableOneComp(prepare_table*)));
    toc->start();

    listen( QHostAddress::Any, port );

    if (port == 8888) m_port = 8880;
    if (port == 7777) m_port = 7770;

    connect( &server, SIGNAL( newConnection() ),this, SLOT( startComm()) );
    server.listen( QHostAddress::Any, m_port );
}

Server::~Server()
{
    for(int i = 0; i < 128; i++)
    {
        key[i] = '\0';
        tableOne->key[i] = '\0';
        tableTwo->key[i] = '\0';
    }

    if(client->isOpen())
    {
        client->close();
    }
    if(p2p->isOpen())
    {
        p2p->close();
    }
    if(server.isListening())
    {
        server.close();
    }
    close();
}

void Server::incomingConnection( qintptr sd )
{
    if(m_crypted == true)
    {
        if(client == NULL)
        {
            client = new QSslSocket();
            client->setSocketDescriptor( sd );

            connect( client, SIGNAL( sslErrors( QList<QSslError> ) ), this, SLOT( HandleError( QList<QSslError> ) ) );

            QSslCertificate server = QSslCertificate::fromPath("../certs/server.crt").first();
            client->setLocalCertificate(server);
            client->setPrivateKey("../certs/server.key");

            connect( client, SIGNAL( encrypted() ), this, SLOT( Ready() ) );
            client->startServerEncryption();
        }
    }
}

void Server::startComm()
{
    std::cout << "Crypted communication established." << std::endl;
    p2p = server.nextPendingConnection();
    connect(p2p, SIGNAL(readyRead() ), this, SLOT (ReadMsg() ) );
}

void Server::ReadMsg()
{
    char buffer[ 1024 ] = {0};
    int length = p2p->read( buffer, p2p->bytesAvailable() );

        if(m_key == false)
        {
            std::copy(key, key + 128, tableOne->key);
            std::copy(key, key + 128, tableTwo->key);
            m_key = true;
        }

        if(prepare1)
        {
            std::cout << "preparing table 1, counter: " << tableOne->counter << std::endl;
            tableOne->counter = tableTwo->counter;

            emit tableOneSignal(tableOne);
            prepare1 = false;
        }
        if(prepare2)
        {
            std::cout << "preparing table 2, counter: " << tableTwo->counter << std::endl;
            tableTwo->counter = tableOne->counter;

            emit tableOneSignal(tableTwo);
            prepare2 = false;
        }

        unsigned char* input = (unsigned char *) malloc(1024 * sizeof(unsigned char));
        unsigned char* output = (unsigned char *) malloc(1024 * sizeof(unsigned char));
        for(int i = 0; i < 1024; i++)
        {
            input[i] = '\0';
            output[i] = '\0';
        }
        memcpy(input,buffer, length);

        unsigned char temp[32];
        unsigned char hash[] = "Testing key";

        sha256_hmac(hash, 11, (const unsigned char*)buffer, length - 32, temp , 0);

        if(using_table)
        {
            xor_table(output, tableOne->p_table, input, length - 32);
        }
        else
        {
            xor_table(output, tableTwo->p_table, input, length - 32);
        }
        counter += length;

        if(counter > (20480 / 2) && using_table && helpbool == true)
        {
            prepare2 = true;
            helpbool = false;
        }

        if(counter > (20480 / 2) && !using_table && helpbool == true)
        {
            prepare1 = true;
            helpbool = false;
        }

        if(counter > (20480 - 1024) && using_table)
        {
            using_table = false;
            counter = 0;
            helpbool = true;
        }

        if(counter > (20480 - 1024) && !using_table)
        {
            using_table = true;
            counter = 0;
            helpbool = true;
        }

        bool testing = true;
        for(int i = 0; i < 32; i++)
        {
            if(temp[i] != (unsigned char)buffer[length - 32 + i])
            {
                testing = false;
            }
        }

        if(!testing)
        {
            std::cout << "Hash doesnt match, msg could be changed." << std::endl;
        }

        std::cout << output << std::endl;
        free(input);
        free(output);
}

void Server::Ready()
{
    qDebug() << "Connected.";
    connect( client, SIGNAL( readyRead() ), this, SLOT( startRead() ) );
    connect( client, SIGNAL(disconnected()), this, SLOT(CloseSSL()));
}

void Server::CloseSSL()
{
    std::cout << "Closing SSL server socket." << std::endl;
    client->close();
    close();
}

void Server::HandleError( QList<QSslError> err )
{

    client->ignoreSslErrors( err );
}

void Server::startRead()
{
    char buffer[ 1024 ] = {0};
    int length = client->read( buffer, client->bytesAvailable() );

   if( !strncmp( buffer, "key1", 4) )
   {

       ctr_drbg_context ctr_drbg;
       entropy_context entropy;

       unsigned char part_key[128];
       char *pers = "random_string";
       int ret;

       entropy_init( &entropy );
       if( ( ret = ctr_drbg_init( &ctr_drbg, entropy_func, &entropy,
           (unsigned char *) pers, strlen( pers ) ) ) != 0 )

       {
           printf( " failed\n ! ctr_drbg_init returned -0x%04x\n", -ret );
           return;
       }

       if( ( ret = ctr_drbg_random( &ctr_drbg, part_key, 128 ) ) != 0 )
       {
           printf( " failed\n ! ctr_drbg_random returned -0x%04x\n", -ret );
           return;
       }

       unsigned char temp[132] = "key2";

       std::copy(part_key, part_key + 128, temp + 4);
       client->write((const char*)temp, 132);

       for(int i = 0; i < 4; i++)
       {
           temp[i] = '\0';
           buffer[i] = '\0';
       }

       for(int i = 0; i < 128; i++)
       {
           key[i] = part_key[i] ^ buffer[i + 4];
           part_key[i] = '\0';
           buffer[i + 4] = '\0';
           temp[i + 4] = '\0';
       }
       std::cout << "Keys exchanged." << std::endl;
   }
}
