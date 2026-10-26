#include "tcpmanger.h"
#include <QNetworkProxy>

Tcpmanger::Tcpmanger(QObject *parent)
    : QObject(parent)
    , m_server(new QTcpServer(this))
    , m_socket(new QTcpSocket(this))
{
    m_socket->setProxy(QNetworkProxy::NoProxy);

    // 客户端收到数据后，通过信号交给主窗口。
    connect(m_socket, &QTcpSocket::readyRead, this, [this]() {
        emit dataReceived(m_socket->readAll());
    });

    // 服务器接收新客户端，沿用原来的列表管理方式。
    connect(m_server, &QTcpServer::newConnection, this, [this]() {
        while (m_server->hasPendingConnections()) {
            QTcpSocket* socket = m_server->nextPendingConnection();
            m_socket_l.append(socket);
            connect(socket, &QTcpSocket::readyRead, this, [this, socket]() {
                emit dataReceived(socket->readAll());
            });
            connect(socket, &QTcpSocket::disconnected, this, [this, socket]() {
                m_socket_l.removeOne(socket);
                socket->deleteLater();
            });
        }
    });
}

Tcpmanger::~Tcpmanger()
{
    stop();
}

void Tcpmanger::start(bool isServerMode, const QString &ip, quint16 port)
{
    stop();
    m_isServerMode = isServerMode;
    m_enabled = true;
    if (m_isServerMode)
        m_server->listen(QHostAddress(ip), port);
    else
        m_socket->connectToHost(ip, port);
}

void Tcpmanger::stop()
{
    m_enabled = false;
    m_server->close();
    m_socket->abort();
    const auto sockets = m_socket_l;
    m_socket_l.clear();
    for (auto socket : sockets)
        socket->abort();
}

void Tcpmanger::send(const QByteArray &data)
{
    if (!m_enabled)
        return;
    // 客户端发给服务器；服务器仍发给最后接入的客户端。
    QTcpSocket* socket = m_isServerMode
            ? (m_socket_l.isEmpty() ? nullptr : m_socket_l.last()) : m_socket;
    if (socket && socket->state() == QAbstractSocket::ConnectedState)
        socket->write(data);
}
