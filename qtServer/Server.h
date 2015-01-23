
#pragma once

#include <QtNetwork>
#include <QObject>
#include <QString>
#include <QTcpSocket>
#include <QtSql/QtSql>
#include <QTime>

#include <map>

#include "../LogFile/logfile.h"
extern LogFile* g_log;
class ClientThread;

struct User
{
    User() {}
    User( std::string ip )
    :   m_ip( ip )
    {}

    std::string m_ip;

};
typedef std::map<std::string, User> UserList;

class Server: public QTcpServer
{
Q_OBJECT

public:
    Server( QObject* parent = 0 );
    virtual ~Server();

    //static void UpdateUserList();

protected:
    void incomingConnection( qintptr socketDescriptor );

public:
    static std::map<std::string, ClientThread*> s_threadList;
    static QSqlDatabase s_db;
    static UserList s_userList;

};
