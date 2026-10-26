#include "tcpphraser.h"
#include <QHash>
#include <QtMath>

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

        parseFrame(body);
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
        // 先检查“字段:数值”格式，避免访问不存在的 pair[1]。
        if (pair.size() != 2 || pair[0].trimmed().isEmpty())
            return;

        bool ok = false;
        const double value = pair[1].toDouble(&ok);
        if (!ok || !qIsFinite(value))
            return; // 拒绝无效数值；合法的 0 仍可通过。

        values[pair[0].trimmed()] = value;
    }

    // 六项必须齐全，避免缺失字段被当成 0 显示或入库。
    const char* keys[] = {"temp", "humi", "light", "soil", "mq2", "rain"};
    for (const char* key : keys) {
        if (!values.contains(key))
            return;
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
