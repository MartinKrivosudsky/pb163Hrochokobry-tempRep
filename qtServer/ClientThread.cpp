
#include <iostream>
#include <string>
#include <sstream>
#include <ctype.h>

#include "ClientThread.h"
#include "../ProtocolMsg.h"

ClientThread::ClientThread( int socketDescriptor, QObject* parent )
:   QThread( parent ),
    m_socket( NULL ),
    m_socketDescriptor( socketDescriptor ),
    m_state( CONNECTED ),
    m_username( "NOT LOGGED IN YET" )
    //m_messages( 2048 )
{
    connect( this, SIGNAL( finished() ), this, SLOT( deleteLater() ) );
}

void ClientThread::deleteLater()
{
    m_socket->close();
}

void ClientThread::run()
{
#ifndef SSL
    m_socket = new QTcpSocket();
    m_socket->setSocketDescriptor( m_socketDescriptor );
    Ready();
#else
    m_socket = new QSslSocket();
    m_socket->setSocketDescriptor( m_socketDescriptor );

    connect( m_socket, SIGNAL( disconnected() ), this, SLOT( LogOut() ) );
    connect( m_socket, SIGNAL( sslErrors( QList<QSslError> ) ), this, SLOT( HandleError( QList<QSslError> ) ) );

    QSslCertificate server = QSslCertificate::fromPath("../certs/server.crt").first();
    m_socket->setLocalCertificate(server);
    m_socket->setPrivateKey("../certs/server.key");

    connect( m_socket, SIGNAL( encrypted() ), this, SLOT( Ready() ) );
    m_socket->startServerEncryption();
#endif
    exec();
}

void ClientThread::Ready()
{
    std::stringstream logStream;
    logStream << LOG_THREAD
              << "IP " << m_socket->peerAddress().toString().toStdString() << ":"
              << m_socket->peerPort() << " connected.";
    g_log->make_log( logStream.str() );

    std::cout << logStream.str() << std::endl;

    m_socket->write( Msg::LoginData.c_str(), Msg::LoginData.size() + 1 );

    logStream.str( std::string() );
    logStream << LOG_THREAD
              << "request for login data sent.";
    g_log->make_log( logStream.str() );

    connect( m_socket, SIGNAL( readyRead() ), this, SLOT( Read() ), Qt::DirectConnection );
}

void ClientThread::Read()
{
    char buffer[ 1024 ] = { 0 };
    if( m_socket->bytesAvailable() < 1024 )
        m_socket->read( buffer, m_socket->bytesAvailable() );
    else
        m_socket->bytesAvailable();

    std::cout << buffer << std::endl;
    //m_messages.FromBuffer( buffer );

    std::string str( buffer );
    ProcessMsg( str );
}

void ClientThread::HandleError( QList<QSslError> errors )
{
    foreach( QSslError error, errors )
    {
        qDebug() << "SSL error: " << error.errorString();
    }
#ifdef SSL
    m_socket->ignoreSslErrors(errors);
#endif
}

void ClientThread::SendMsg( const std::string& msg )
{
    m_socket->write( msg.c_str(), msg.size() + 1 );
}

void ClientThread::ProcessMsg( std::string& str )
{
    std::stringstream msg( str );
    std::string action;
    std::string header;
    std::string value;
    switch( m_state )
    {
    case CONNECTED:
        msg >> action;
        if( !action.compare( "ld" ) )
        {
            HandleLogin( msg );
            return;
        }
        else if( !action.compare( "reg" ) )
        {
            HandleRegistration( msg );
            return;
        }
        break;
    case LOGGED:
        msg >> header;
        if( !header.compare( "rul" ) )
        {
            SendUserList();
            return;
        }
        if( !header.compare( "ud" ) )
        {
            std::string username;
            msg >> username;
            SendUserDetails( username );
            break;
        }
        else if( !header.compare( "lo" ) )
        {
            LogOut();
            return;
        }
        else if( !header.compare( "t" ) )
        {
            msg >> value;
            std::reverse( value.begin(), value.end() );
            m_socket->write( "t ", 2 );
            m_socket->write( value.c_str(), value.size() + 1 );
            return;
        }
        break;
    }
}


void ClientThread::HandleLogin( std::stringstream& msg )
{
    std::stringstream logStream;

    std::string nameHeader;
    std::string passHeader;
    std::string name;
    std::string pass;
    msg >> nameHeader >> name >> passHeader >> pass;
    if( nameHeader.compare( "u" ) || !name.size() || passHeader.compare( "p" ) || !pass.size() )
    {
        logStream << "Invalid packet for login.";
        g_log->make_log( logStream.str() );

        std::cout << logStream << std::endl;

        m_socket->write( "e 1 ", 4 );
        m_socket->flush();
        m_socket->write( Msg::LoginData.c_str(), Msg::LoginData.size() + 1 );

        return;
    }

    QSqlQuery query( Server::s_db );
    std::string q( "SELECT password FROM users WHERE username = '" );
    q.append( name + "'" );

    if( query.exec( q.c_str() ) )
    {
        if( query.next() )
        {
            if( !pass.compare( query.value( "password" ).toString().toStdString() ) )
            {
                m_username = name;
                std::stringstream ip;
                ip << m_socket->peerAddress().toString().toStdString();
                Server::s_userList[ m_username ] = User( ip.str() );

                m_socket->write( "li ", 4 );
                m_state = LOGGED;

                logStream << LOG_THREAD <<
                          "user: '" << m_username << "' logged in.";
                g_log->make_log( logStream.str() );

                Server::s_threadList[ m_username ] = (ClientThread*)this;

                return;
            }
            else
            {
                logStream << "Error on login: username: '" << name << "' has invalid password.";
                g_log->make_log( logStream.str() );

                std::cout << logStream << std::endl;
            }
        }
        else
        {
            logStream << "Error on login: invalid username: '" << name << "'.";
            g_log->make_log( logStream.str() );

            std::cout << logStream << std::endl;
        }
    }
    else
    {
        logStream << "Error processing query on login of '" << name << "' : " << q
                  << " | " << query.lastError().text().toStdString() << ".";
        g_log->make_log( logStream.str() );

        std::cout << logStream << std::endl;
    }

    m_socket->write( "e 1 ", 4 );
    m_socket->flush();
    m_socket->write( Msg::LoginData.c_str(), Msg::LoginData.size() + 1 );
}

void ClientThread::HandleRegistration( std::stringstream& msg )
{
    std::stringstream logStream;

    std::string nameHeader;
    std::string passHeader;
    std::string name;
    std::string pass;
    msg >> nameHeader >> name >> passHeader >> pass;
    if( nameHeader.compare( "u" ) || !name.size() ||
        passHeader.compare( "p" ) || !pass.size() )
    {
        logStream << "Invalid packet for registration.";
        g_log->make_log( logStream.str() );

        std::cout << logStream << std::endl;

        m_socket->write( "e 1 ", 4 );
        m_socket->flush();
        m_socket->write( Msg::LoginData.c_str(), Msg::LoginData.size() + 1 );

        return;
    }

    QSqlQuery lookUpQuery( Server::s_db );
    std::string lq( "SELECT password FROM users WHERE username = '" );
    lq.append( name + "'" );

    if( lookUpQuery.exec( lq.c_str() ) )
    {
        if( lookUpQuery.next() )
        {
            logStream << "Error on registration of '" << name << "' : name already taken.";
            g_log->make_log( logStream.str() );

            std::cout << logStream << std::endl;

            m_socket->write( "e 2 ", 4 );
            m_socket->flush();
            m_socket->write( Msg::LoginData.c_str(), Msg::LoginData.size() + 1 );

            return;
        }
    }
    else
    {
        logStream << "Error processing LOOKUP query on registration of '" << name << "' : " << lq
                  << " | " << lookUpQuery.lastError().text().toStdString() << ".";
        g_log->make_log( logStream.str() );

        std::cout << logStream << std::endl;

        m_socket->write( "e 1 ", 4 );
        m_socket->flush();
        m_socket->write( Msg::LoginData.c_str(), Msg::LoginData.size() + 1 );

        return;
    }

    QSqlQuery regQuery( Server::s_db );
    std::string rq( "INSERT INTO users VALUES( '" );
    rq.append( name + "', '" + pass + "' )" );

    if( regQuery.exec( rq.c_str() ) )
    {
        logStream << "Registration of '" << name << "' was successful.";
        g_log->make_log( logStream.str() );

        std::cout << logStream << std::endl;

        logStream << LOG_THREAD
                  << "request for login data sent.";
        m_socket->write( Msg::LoginData.c_str(), Msg::LoginData.size() + 1 );
    }
    else
    {
        logStream << "Error processing query on registration of '" << name << "' : " << lq
                  << " | " << lookUpQuery.lastError().text().toStdString() << ".";
        g_log->make_log( logStream.str() );

        std::cout << logStream << std::endl;

        m_socket->write( "e 1 ", 4 );
        m_socket->flush();
        m_socket->write( Msg::LoginData.c_str(), Msg::LoginData.size() + 1 );

        return;
    }
}

void ClientThread::SendUserList()
{
    std::stringstream logStream;

    logStream << LOG_THREAD <<
              "requested user list.";
    g_log->make_log( logStream.str() );

    m_socket->write( "ul ", 3 );
    for( UserList::const_iterator i = Server::s_userList.begin(), e = Server::s_userList.end(); i != e; i++ )
    {
        if( i->first.compare( m_username ) )    // leave out user to whom the list will be sent
        {
            m_socket->write( "u ", 2 );
            m_socket->write( i->first.c_str(), i->first.size() );
            m_socket->write( " ", 1 );
        }
    }
    m_socket->write( "", 1 );
}

void ClientThread::SendUserDetails( string& username )
{
    UserList::const_iterator user = Server::s_userList.find( username );
    if( user != Server::s_userList.end() )
    {
        m_socket->write( "ud ", 3 );
        m_socket->write( "ip ", 3 );
        m_socket->write( user->second.m_ip.c_str(), user->second.m_ip.size() );
        m_socket->write( " ", 1 );
        m_socket->write( "", 1 );
    }
    else
    {
        m_socket->write( "e 5 ", 5 );
    }
}

void ClientThread::LogOut()
{
    std::stringstream logStream;
    logStream << LOG_THREAD <<
              m_username << " logged out.";
    g_log->make_log( logStream.str() );

    logStream.str( std::string( "" ) );
    logStream << LOG_THREAD << "threat ended.";
    g_log->make_log( logStream.str() );

    Server::s_userList.erase( m_username );
    Server::s_threadList.erase( m_username );

    quit();
}
