
#pragma once

#include <QtNetwork>
#include <QObject>
#include <QString>
#include <QTcpSocket>
#include "../crypto_crt/crypto.hpp"

extern unsigned char key[128];

#define echo std::cout << __FILE__ << ": " << __LINE__ << " " << std::endl;

class Server: public QTcpServer
{
Q_OBJECT

public:
    Server( int port, QObject * parent = 0 );
    ~Server();

public slots:
    void startRead();
    void Ready();
    void HandleError( QList<QSslError> );
    void CloseSSL();
    void startComm();
    void ReadMsg();
signals:
    void tableOneSignal(prepare_table*);
protected:
    void incomingConnection( qintptr sd );

private:
    QSslSocket* client;
    QTcpSocket* p2p;
    QTcpServer server;
    prepare_table* tableOne;
    prepare_table* tableTwo;
    bool m_key;
    int m_port;
    bool m_crypted;
    int counter;
    bool prepare1;
    bool prepare2;
    bool using_table;
    bool helpbool;

};
