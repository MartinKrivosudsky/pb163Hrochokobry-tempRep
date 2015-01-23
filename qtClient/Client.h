
#pragma once

#include <QtNetwork>
#include <QObject>
#include <QString>
#include <QTcpSocket>
#include <QSslSocket>
#include "../crypto_crt/crypto.hpp"
#define STR_IP( ip ) ip.toString().toStdString()

class Client : public QObject
{
    Q_OBJECT
public:
    Client(QObject* parent = NULL );
    Client( bool peer, QObject* parent = NULL);
    ~Client();

    void start( QString address, quint16 port );
    void startSSL( QString address, quint16 port );
    void newConnection();
public slots:
    void ConnectedClient();
    void SendMessage(std::string);
    void ReceiveData();
    void ReceiveDataClient();
    void HandleError( QAbstractSocket::SocketError socketError );
    void SslError( QList<QSslError> errors );
    void SslErrorClient( QList<QSslError> errors );
    void Encrypted();
    void EncryptedClient();
    void Disconnected();
    void ConnectToPeer(std::string ip);
    void ConnectedEst();
signals:
    void tableOneSignal(prepare_table*);
private:
    QSslSocket m_SC;
    QSslSocket m_p2pSSL;
    QTcpSocket m_p2p;
    bool m_connected;
    bool m_peer;
    bool m_key;
    int counter;
    bool prepare1;
    bool prepare2;
    bool using_table;
    bool helpbool;
    prepare_table* tableOne;
    prepare_table* tableTwo;
    std::string m_name;
    std::string m_pass;
    unsigned char part_key[128];
    QString m_address;
    quint16 m_port;

    enum SocketError {
        ConnectionRefusedError,
        RemoteHostClosedError,
        HostNotFoundError,
        SocketAccessError,
        SocketResourceError,
        SocketTimeoutError,                     /* 5 */
        DatagramTooLargeError,
        NetworkError,
        AddressInUseError,
        SocketAddressNotAvailableError,
        UnsupportedSocketOperationError,        /* 10 */
        UnfinishedSocketOperationError,
        ProxyAuthenticationRequiredError,
        SslHandshakeFailedError,
        ProxyConnectionRefusedError,
        ProxyConnectionClosedError,             /* 15 */
        ProxyConnectionTimeoutError,
        ProxyNotFoundError,
        ProxyProtocolError,
        OperationError,
        SslInternalError,                       /* 20 */
        SslInvalidUserDataError,
        TemporaryError,

        UnknownSocketError = -1
    };
};
