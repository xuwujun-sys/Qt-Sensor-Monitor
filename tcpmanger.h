#ifndef TCPMANGER_H
#define TCPMANGER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QVector>

// 负责 TCP 收发和连接管理，不依赖界面或传感器协议。
class Tcpmanger : public QObject
{
    Q_OBJECT

public:
    explicit Tcpmanger(QObject *parent = nullptr); // 创建服务端和客户端对象，连接收发事件。
    ~Tcpmanger() override; // 停止监听并断开所有连接。

    void start(bool isServerMode, const QString &ip, quint16 port); // 先停止旧连接，true 监听，false 主动连接。
    void stop(); // 关闭监听、断开客户端连接并清空客户端列表。
    void send(const QByteArray &data); // 联网且已连接时发送；服务器发给最后接入的客户端。

signals:
    // 收到原始字节时发出；不保证是一整帧，由解析器处理拆包和粘包。
    void dataReceived(QByteArray data);

private:
    bool m_isServerMode = false; // 当前启动角色：false 客户端，true 服务器。
    bool m_enabled = false; // 是否请求开启网络，不代表连接或监听成功。
    QTcpServer* m_server; // 服务器模式下监听端口、接受连接。
    QTcpSocket* m_socket; // 客户端模式下与服务器通信。
    QVector<QTcpSocket*> m_socket_l; // 服务器模式下接入的客户端。
};

#endif // TCPMANGER_H
