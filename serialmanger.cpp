#include "serialmanger.h"
#include <QSerialPortInfo>

SerialManger::SerialManger(QObject *parent)
    : QObject(parent)
    , m_serial(new QSerialPort(this))
{
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
    QStringList names;
    for (const auto &port : QSerialPortInfo::availablePorts())
        names.append(port.portName());
    return names;
}

bool SerialManger::open(const QString &portName, int baudRate)
{
    close();
    m_serial->setPortName(portName);
    // 与串口配置页一致：8 位数据、无校验、1 位停止位，无流控。
    if (!m_serial->setBaudRate(baudRate)
            || !m_serial->setDataBits(QSerialPort::Data8)
            || !m_serial->setParity(QSerialPort::NoParity)
            || !m_serial->setStopBits(QSerialPort::OneStop)
            || !m_serial->setFlowControl(QSerialPort::NoFlowControl))
        return false;
    return m_serial->open(QIODevice::ReadWrite);
}

void SerialManger::close()
{
    m_serial->close();
}

void SerialManger::send(const QByteArray &data)
{
    if (m_serial->isOpen())
        m_serial->write(data);
}
