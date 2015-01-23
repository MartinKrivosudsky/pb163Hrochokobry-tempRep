#include "client.h"
#include "server.h"
#include <iostream>
#include <QHostAddress>
#include <QThread>
#include <time.h>
#include "inputthread.h"
#include "../ProtocolMsg.h"
#include "../crypto_crt/crypto.hpp"
#include "tableone.h"
#include <unistd.h>

#include "polarSSL/sha256.h"
#include "polarssl/entropy.h"
#include "polarssl/ctr_drbg.h"

unsigned char key[128];

Client::Client( QObject* parent )
:   QObject( parent ),
    m_connected( false )
{
    connect( &m_SC, SIGNAL( disconnected() ), this, SLOT( Disconnected() ));
    m_SC.startClientEncryption();
}

void Client::Disconnected()
{
    qDebug() << "Dced from server.";
}

Client::Client(bool peer, QObject* parent )
:   QObject( parent ),
    m_connected( false ),
    m_peer(peer),
    m_key(false),
    counter(0),
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
}
Client::~Client()
{
    if(m_SC.isOpen())
    {
        m_SC.write("lo", 3);
        m_SC.close();
    }
    if(m_p2p.isOpen())
    {
        m_p2p.close();
    }
    if(m_p2pSSL.isOpen())
    {
        m_p2pSSL.close();
    }

    for(int i = 0; i < 128; i++)
    {
        key[i] = '\0';
        tableOne->key[i] = '\0';
        tableTwo->key[i] = '\0';
    }

    delete tableOne;
    delete tableTwo;
}

void Client::start( QString address, quint16 port )
{
    m_address = address;
    m_port = port;

    QHostAddress addr( address );

    QSslCertificate clientCert = QSslCertificate::fromPath("../certs/client.crt").first();
    if (clientCert.isNull())
    {
        qDebug() << "Failed to load CLIENT certificate";
    }
    m_p2pSSL.setLocalCertificate(clientCert);
    m_p2pSSL.setPrivateKey("../certs/client.key");

    connect( &m_p2pSSL, SIGNAL( encrypted() ), this, SLOT( EncryptedClient() ) );
    connect( &m_p2pSSL, SIGNAL( sslErrors(QList<QSslError>) ), this, SLOT(SslErrorClient(QList<QSslError> ) ) );

    m_p2pSSL.connectToHostEncrypted( address, port );
}

void Client::startSSL( QString address, quint16 port )
{
    QHostAddress addr( address );

    QSslCertificate clientCert = QSslCertificate::fromPath("../certs/client.crt").first();
    if (clientCert.isNull())
    {
        qDebug() << "Failed to load CLIENT certificate";
    }
    m_SC.setLocalCertificate(clientCert);
    m_SC.setPrivateKey("../certs/client.key");

    connect( &m_SC, SIGNAL( encrypted() ), this, SLOT( Encrypted() ) );
    connect( &m_SC, SIGNAL( sslErrors(QList<QSslError>) ), this, SLOT(SslError(QList<QSslError> ) ) );

    m_SC.connectToHostEncrypted( address, port );
}

void Client::Encrypted()
{
    connect( &m_SC, SIGNAL( readyRead() ), this, SLOT( ReceiveData() ) );
    qDebug() << "Connected to server.";
    qDebug() << "Encryption working.";
    m_connected = true;
}

void Client::EncryptedClient()
{
    ConnectedClient();
    connect( &m_p2pSSL, SIGNAL( readyRead() ), this, SLOT( ReceiveDataClient() ) );
    qDebug() << "Connected to client.";
    qDebug() << "Encryption working.";
}

void Client::ConnectedClient()
{

    std::cout << "Connected to: " << STR_IP( m_p2pSSL.peerAddress() ) << ":"
              << m_p2pSSL.peerPort() << "." << std ::endl;
    m_connected = true;

    if(m_peer == true)
    {
        ctr_drbg_context ctr_drbg;
        entropy_context entropy;

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

        unsigned char temp[132] = "key1";


        std::copy(part_key, part_key + 128, temp + 4);
        m_p2pSSL.write((const char*)temp, 132);

        for(int i = 0; i < 132; i++)
        {
            temp[i] = '\0';
        }
    }
    else
    {
        std::cout << "Closing SSL client socket." << std::endl;
        m_p2pSSL.close();
        newConnection();
    }
}

void Client::newConnection()
{
    QHostAddress addr( m_address );

    connect( &m_p2p, SIGNAL( connected() ), this, SLOT( ConnectedEst() ) );
    connect( &m_p2p, SIGNAL( error( QAbstractSocket::SocketError ) ),
                 this, SLOT( HandleError( QAbstractSocket::SocketError ) ) );


    if(m_port == 8888) m_p2p.connectToHost(addr, 8880);
    if(m_port == 7777) m_p2p.connectToHost(addr, 7770);

}

void Client::ConnectedEst()
{
    std::cout << "Connected to client without SSL." << std::endl;

    m_connected = true;
    InputThread * thread = new InputThread(this);
    connect(thread, SIGNAL(inputSignal(std::string) ), this, SLOT (SendMessage(std::string)));
    thread->start();
}

void Client::ReceiveDataClient()
{
    char buffer[ 1024 ] = { 0 };
    m_p2pSSL.read( buffer, m_p2pSSL.bytesAvailable() );

    if( !strncmp( buffer, "key2", 4) )
    {
        for(int i = 0; i < 128; i++)
        {
            key[i] = buffer[i + 4] ^ part_key[i];
            part_key[i] = '\0';
            buffer[i + 4] = '\0';
        }
        std::cout << "Keys exchanged." << std::endl;
        std::cout << "Closing SSL client socket." << std::endl;
        m_p2pSSL.close();
        newConnection();
    }
}

void Client::ReceiveData()
{
    char buffer[ 1024 ] = { 0 };
    m_SC.read( buffer, m_SC.bytesAvailable() );

    if( !strncmp( buffer, "ld", 2) )
    {
        std::cout << "Press 1 for register 2 for login" << std::endl;
        std::string menu;
        std::getline(std::cin, menu);

        std::cout << "Enter your name: ";
        std::getline(std::cin, m_name);
        std::cout << "Password: ";
        std::getline(std::cin, m_pass);

        if( !strncmp( menu.c_str(), "1", 1) )
        {
            std::string buffer;
            buffer = Msg::RegisterNewUser +" "+ Msg::UsernameHeader +" "+ m_name +" "+ Msg::PasswordHeader +" "+ m_pass;
            m_SC.write(buffer.c_str(), buffer.size());
        }
        else if( !strncmp( menu.c_str(), "2", 1) )
        {
            std::string buffer;
            buffer = Msg::LoginData +" "+ Msg::UsernameHeader +" "+ m_name +" "+ Msg::PasswordHeader +" "+ m_pass;
            m_SC.write(buffer.c_str(), buffer.size());
        }
        else
        {
            std::cout << "Invalid command" << std::endl;
        }
    }
    else if(!strncmp (buffer, "li", 2))
    {
         m_SC.write( "rul ", 5 );
    }
    else if(!strncmp (buffer, "ul", 2))
    {
        std::string temp(buffer, 3, strlen(buffer) - 2);
        std::cout << temp << std::endl;
        std::cout << "Choose user: (type 'refresh' to refresh list or 'dc' to disconnect) " << std::endl;
        std::string user;
        std::getline(std::cin, user);
        if(!strncmp (user.c_str(), "refresh", 7))
        {
            m_SC.write( "rul ", 5 );
        }
        else if(!strncmp (user.c_str(), "dc", 2))
        {
            m_SC.write( "lo ", 5 );
            m_SC.close();
        }
        else
        {
            user = Msg::RequestUserDetails +" "+ user;
            m_SC.write(user.c_str(), user.size()+1);
        }
    }
    else if(!strncmp (buffer, "ud ip", 5))
    {
        std::string ip(buffer, 6, strlen(buffer) - 5);
        ConnectToPeer(ip);
    }
    else if(!strncmp (buffer, "e 1 ", 4))
    {
        std::cout << "Wrong name or password" << std::endl;
    }
    else if(!strncmp (buffer, "e 2 ", 4))
    {
        std::cout << "User already exists" << std::endl;
    }
    else if(!strncmp (buffer, "e 5 ", 4))
    {
        std::cout << "User doesnt exists" << std::endl;
        m_SC.write("rul ", 5);
    }
    else
    {
        std::cout << "Server refused connection" << std::endl;
        m_SC.close();
    }
}

void Client::HandleError( QAbstractSocket::SocketError error )
{
    switch( error )
    {
    case ConnectionRefusedError:
        std::cout << "ConnectionRefusedError." << std::endl;
        break;
    case RemoteHostClosedError:
        std::cout << "RemoteHostClosedError." << std::endl;
        break;
    case HostNotFoundError:
        std::cout << "HostNotFoundError." << std::endl;
        break;
    case SocketTimeoutError:
        std::cout << "SocketTimeoutError." << std::endl;
        break;
    case NetworkError:
        std::cout << "NetworkError." << std::endl;
        break;
    case SslHandshakeFailedError:
        std::cout << "SslHandshakeFailedError." << std::endl;
        break;
    default:
        std::cout << "Unknown error while connecting." << std::endl;
        break;
    }
}

void Client::SslError( QList<QSslError> errors )
{
    m_SC.ignoreSslErrors( errors );
}

void Client::SslErrorClient( QList<QSslError> errors )
{
    m_p2pSSL.ignoreSslErrors( errors );
}

void Client::SendMessage(std::string line)
{
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

    memcpy(input,line.c_str(), line.size());
    if(using_table)
    {
        xor_table(output, tableOne->p_table, input, line.size());
    }
    else
    {
        xor_table(output, tableTwo->p_table, input, line.size());
    }
    counter += line.size();

    if(counter > (20480 / 2) && using_table && helpbool == true)
    {
        prepare2 = true;
        helpbool = false;
    }

    if(counter > (20480 / 2) && !using_table &&helpbool == true)
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

    unsigned char hash[] = "Testing key";
    sha256_hmac(hash, 11, (const unsigned char*)output, line.size(),output + line.size() , 0);

    m_p2p.write( (const char*)output, line.size() + 32 );
    free(input);
    free(output);
}

void Client::ConnectToPeer(std::string ip)
{
    Client * client2 = new Client(true);
    client2->start(ip.c_str(), 8888);
}
