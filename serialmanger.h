#ifndef SERIALMANGER_H
#define SERIALMANGER_H

#include <QObject>
#include <QSerialPort>
#include <QStringList>

// 负责串口收发，不操作界面，也不解析传感器协议。
class SerialManger : public QObject
{
    Q_OBJECT

public:
    explicit SerialManger(QObject *parent = nullptr); // 创建串口对象，连接接收事件。
    ~SerialManger() override; // 关闭串口，串口对象由 Qt 自动释放。

    QStringList portNames() const; // 获取本机当前可用的串口名称。
    bool open(const QString &portName, int baudRate); // 按固定 8N1 参数打开，成功返回 true。
    void close(); // 关闭串口。
    void send(const QByteArray &data); // 串口打开时发送原始字节。

signals:
    void dataReceived(QByteArray data); // 收到原始字节时发出，完整报文由解析器判断。

private:
    QSerialPort* m_serial; // 管理一个串口的配置与收发。
};

#endif // SERIALMANGER_H
