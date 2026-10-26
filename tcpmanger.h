#ifndef TCPMANGER_H
#define TCPMANGER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QVector>

class Tcpmanger : public QObject
{
    Q_OBJECT

public:
    explicit Tcpmanger(QObject *parent = nullptr);
    ~Tcpmanger() override;

    void start(bool isServerMode, const QString &ip, quint16 port);
    void stop();
    void send(const QByteArray &data);

signals:
    // 只把原始数据交给主窗口，不操作界面或解析协议。
    void dataReceived(QByteArray data);

private:
    bool m_isServerMode = false;
    bool m_enabled = false;
    QTcpServer* m_server;
    QTcpSocket* m_socket;
    QVector<QTcpSocket*> m_socket_l; // 服务器模式下接入的客户端。
};

#endif // TCPMANGER_H
