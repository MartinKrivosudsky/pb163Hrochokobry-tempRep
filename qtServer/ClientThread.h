
#pragma once

#define SSL

#include <QObject>
#include <QThread>
#include <QTcpSocket>
#include <QTime>

#include "Server.h"

#define LOG_THREAD "Thread: " << currentThreadId() << " | "

class ClientThread : public QThread
{
Q_OBJECT

    enum State
    {
        CONNECTED,
        LOGGED
    };

public:
    ClientThread( int socketDescriptor, QObject* parent = NULL );

    void run();
    void SendMsg( const std::string& msg );

public slots:
    void Ready();
    void Read();
    void deleteLater();
    void HandleError( QList<QSslError> errors );
    void LogOut();

protected:
    void ProcessMsg( std::string& str );

    void HandleLogin( std::stringstream& msg );
    void HandleRegistration( std::stringstream& msg );
    void SendUserList();
    void SendUserDetails( std::string& username );

private:
#ifndef SSL
    QTcpSocket* m_socket;
#else
    QSslSocket* m_socket;
#endif
    int m_socketDescriptor;
    State m_state;

    std::string m_username;

};


