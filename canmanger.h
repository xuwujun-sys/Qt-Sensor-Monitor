#ifndef CANMANGER_H
#define CANMANGER_H

#include <QObject>
#include <QCanBusDevice>
#include <QCanBusFrame>
#include <QStringList>

// 管理一个 CAN 通道，按帧收发，不操作界面或解析传感器数据。
class CanManger : public QObject
{
    Q_OBJECT

public:
    explicit CanManger(QObject *parent = nullptr); // 创建管理对象，设备在 open() 时创建。
    ~CanManger() override; // 关闭并释放 CAN 设备。

    QStringList plugins() const; // 获取当前可用的 Qt CAN 插件名称。
    bool open(const QString &plugin, const QString &interface, int bitRate = 500000); // 按插件和通道打开设备。
    void close(); // 断开并释放设备，可重复调用。
    bool send(const QCanBusFrame &frame); // 提交发送帧，返回底层接口的发送结果。

signals:
    void frameReceived(QCanBusFrame frame); // 收到一帧时发出，保留 ID、数据和帧类型。

private:
    QCanBusDevice* m_device = nullptr; // 当前 CAN 通道，未打开时为空。
};

#endif // CANMANGER_H
