#include "tcpphraser.h"
#include <QHash>

TcpPhraser::TcpPhraser(QObject *parent) : QObject(parent)
{
    m_buffer = QByteArray();
}

void TcpPhraser::append_data(const QByteArray data)
{
    m_buffer.append(data);
}

void TcpPhraser::reset()
{
    m_buffer.clear();
}

void TcpPhraser::analyze()
{
    const QByteArray header = "Params{";

    while (true)
    {
        int start = m_buffer.indexOf(header);

        if (start == -1)
        {
            // 保留末尾，避免消息头恰好被分成两次收到
            m_buffer = m_buffer.right(header.size() - 1);
            return;
        }

        // 去掉消息头前面的无关数据
        if (start > 0)
            m_buffer.remove(0, start);

        int end = m_buffer.indexOf('}', header.size());

        if (end == -1)
            return;  // 消息还没收完整，等下次追加

        QByteArray body = m_buffer.mid(
            header.size(), end - header.size());

        m_buffer.remove(0, end + 1);

        parseFrame(body);  // 解析这一条的六个字段
    }
}

void TcpPhraser::parseFrame(const QByteArray &body)
{
    QHash<QByteArray, double> values;

    for (const QByteArray &field : body.split(','))
    {
        if (field.isEmpty())
            continue;  // 跳过末尾逗号产生的空项

        const auto pair = field.split(':');
        values[pair[0].trimmed()] = pair[1].toDouble();
    }

    emit sensorDataParsed(
        values.value("temp"),
        values.value("humi"),
        values.value("light"),
        values.value("soil"),
        values.value("mq2"),
        values.value("rain")
    );
}
