#include "canmanger.h"
#include <QCanBus>
#include <QVariant>

// CAN 收发封装目前未接入 MainWindow；此文件提供独立的通道和帧操作能力。
CanManger::CanManger(QObject *parent) : QObject(parent)
{
}

CanManger::~CanManger()
{
    close();
}

QStringList CanManger::plugins() const
{
    return QCanBus::instance()->plugins();
}

bool CanManger::open(const QString &plugin, const QString &interface, int bitRate)
{
    // plugin 指 Qt CAN 后端，interface 指该后端的设备通道；重新打开前先释放旧设备。
    close();
    m_device = QCanBus::instance()->createDevice(plugin, interface);
    if (!m_device)
        return false;

    m_device->setParent(this);
    m_device->setConfigurationParameter(QCanBusDevice::BitRateKey, bitRate);
    connect(m_device, &QCanBusDevice::framesReceived, this, [this]() {
        // 一次通知可能收到多帧，逐帧取出。
        while (m_device && m_device->framesAvailable() > 0) {
            const QCanBusFrame frame = m_device->readFrame();
            emit frameReceived(frame);
        }
    });

    return m_device->connectDevice();
}

void CanManger::close()
{
    if (!m_device)
        return;
    // 先断开信号，避免设备关闭过程中继续回调；deleteLater 延后安全释放。
    disconnect(m_device, nullptr, this, nullptr);
    m_device->disconnectDevice();
    m_device->deleteLater();
    m_device = nullptr;
}

bool CanManger::send(const QCanBusFrame &frame)
{
    return m_device && m_device->writeFrame(frame);
}
