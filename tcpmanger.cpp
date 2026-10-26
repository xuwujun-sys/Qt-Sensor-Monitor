#include "tcpmanger.h"
#include <QNetworkProxy>

Tcpmanger::Tcpmanger(QObject *parent)
    : QObject(parent)
    , m_server(new QTcpServer(this))
    , m_socket(new QTcpSocket(this))
{
    m_socket->setProxy(QNetworkProxy::NoProxy);

    connect(m_socket, &QTcpSocket::readyRead, this, [this]() {
        emit dataReceived(m_socket->readAll());
    });

    // 为每个客户端连接接收和断开事件，断开后移除并释放。
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
    // 先关闭旧连接，再按指定角色启动。
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
    // 服务器统一发给最后接入的客户端，不按设备类型区分。
    QTcpSocket* socket = m_isServerMode
            ? (m_socket_l.isEmpty() ? nullptr : m_socket_l.last()) : m_socket;
    if (socket && socket->state() == QAbstractSocket::ConnectedState)
        socket->write(data);
}
