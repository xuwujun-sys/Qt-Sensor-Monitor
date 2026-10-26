#include "tcpphraser.h"
#include <QHash>
#include <QtMath>

TcpPhraser::TcpPhraser(QObject *parent) : QObject(parent)
{
    m_buffer = QByteArray();
}

void TcpPhraser::append_data(const QByteArray data)
{
    // TCP 是字节流：新数据可能只是半条报文，也可能包含多条报文。
    m_buffer.append(data);
}

void TcpPhraser::reset()
{
    m_buffer.clear();
}

void TcpPhraser::analyze()
{
    // 协议示例：Params{temp:25,humi:60,light:50,soil:40,mq2:0,rain:0}
    // 本函数负责找报文边界，字段校验交给 parseFrame()。
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

        // 消费当前完整报文，循环继续处理后面可能粘在一起的报文。
        m_buffer.remove(0, end + 1);

        parseFrame(body);
    }
}

void TcpPhraser::parseFrame(const QByteArray &body)
{
    // 用字段名查值，因此六个字段不必按固定顺序排列。
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

    // 所有必需字段通过校验才通知主窗口，错误报文不会更新标签或写入数据库。
    emit sensorDataParsed(
        values.value("temp"),
        values.value("humi"),
        values.value("light"),
        values.value("soil"),
        values.value("mq2"),
        values.value("rain")
    );
}
