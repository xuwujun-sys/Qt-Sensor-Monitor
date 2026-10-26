#include "serialmanger.h"
#include <QSerialPortInfo>

SerialManger::SerialManger(QObject *parent)
    : QObject(parent)
    , m_serial(new QSerialPort(this))
{
    // readyRead 表示当前有字节可读，不保证一次收到完整消息。
    connect(m_serial, &QSerialPort::readyRead, this, [this]() {
        emit dataReceived(m_serial->readAll());
    });
}

SerialManger::~SerialManger()
{
    close();
}

QStringList SerialManger::portNames() const
{
    // 实时扫描系统设备，不缓存；刷新列表时能发现新插入或拔出的串口。
    QStringList names;
    for (const auto &port : QSerialPortInfo::availablePorts())
        names.append(port.portName());
    return names;
}

bool SerialManger::open(const QString &portName, int baudRate)
{
    // 重开前释放旧端口，再设置新的端口名和参数。
    close();
    m_serial->setPortName(portName);
    // 与串口配置页一致：8 位数据、无校验、1 位停止位，无流控。
    if (!m_serial->setBaudRate(baudRate)
            || !m_serial->setDataBits(QSerialPort::Data8)
            || !m_serial->setParity(QSerialPort::NoParity)
            || !m_serial->setStopBits(QSerialPort::OneStop)
            || !m_serial->setFlowControl(QSerialPort::NoFlowControl))
        return false;
    // 以双向模式打开；端口不存在或被占用等情况会返回 false。
    return m_serial->open(QIODevice::ReadWrite);
}

void SerialManger::close()
{
    m_serial->close();
}

void SerialManger::send(const QByteArray &data)
{
    // write() 将字节交给 Qt 发送缓冲区，不代表设备已经接收或处理完成。
    if (m_serial->isOpen())
        m_serial->write(data);
}
