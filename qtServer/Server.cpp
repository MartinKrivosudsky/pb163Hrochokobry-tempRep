
#include <iostream>
#include <sstream>

#include "Server.h"
#include "Console.h"
#include "ClientThread.h"

QSqlDatabase Server::s_db = QSqlDatabase::addDatabase( "QSQLITE" );
std::map<std::string, ClientThread*> Server::s_threadList;
UserList Server::s_userList;

Server::Server( QObject* parent )
:   QTcpServer( parent )
{
    qRegisterMetaType<QList<QSslError> >( "QList<QSslError>" );

    listen( QHostAddress::Any, 8888 );

    g_log->make_log( "Server started." );

    std::cout << "Server started at: " << serverAddress().toString().toStdString()
              << ":" << serverPort() << "." << std::endl;

    s_db.setDatabaseName( "users.db3" );
    s_db.open();

    QSqlQuery query( s_db );
    if( query.exec( "CREATE TABLE users( username text, password text )" ) )
        std::cout << "Table created." << std::endl;
    else
        std::cout << "Table creation warning: " << query.lastError().text().toStdString() << std::endl;

   /* QSqlQuery query2( s_db );
    if( query2.exec( "INSERT INTO users VALUES( 'User1', '123456', '2asg1458ah49had' )" ) )
        std::cout << "Query insterd." << std::endl;
    else
        std::cout << "Table creation warning: " << query.lastError().text().toStdString() << std::endl;*/

    Console* c = new Console( this );
    c->start();
}

Server::~Server()
{
    g_log->make_log( "Server shutdown." );
    close();
}

void Server::incomingConnection( qintptr socketDescriptor )
{
    std::stringstream logStream;
    logStream << "Connection on socket " << socketDescriptor << ".";
    g_log->make_log( logStream.str() );

    ClientThread* thread = new ClientThread( socketDescriptor );;

    thread->start();
}

/*void Server::UpdateUserList()
{
    QTime curTime = QTime::currentTime();
    std::vector<ClientThread*> temp;
    for( std::map<std::string, ClientThread*>::iterator i = s_threadList.begin(), e = s_threadList.end(); i != e; i++ )
        temp.push_back( i->second );

    for( std::vector<ClientThread*>::iterator i = temp.begin(), e = temp.end(); i != e; i++ )
    {
        QTime time = (*i)->GetLastMsgTime();
        time = time.addSecs( 30 );
        if( time < curTime )
        {
            (*i)->LogOut();
        }
    }
}*/

