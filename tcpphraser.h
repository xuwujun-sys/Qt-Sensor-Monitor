#ifndef TCPPHRASER_H
#define TCPPHRASER_H

#include <QObject>
#include "QString"
#include "QByteArray"

// 缓存 TCP 字节流，仅对完整且有效的 Params{...} 报文发出解析结果。
class TcpPhraser : public QObject
{
    Q_OBJECT
public:
    explicit TcpPhraser(QObject *parent = nullptr); // 创建解析器，初始化接收缓存。
    void append_data(const QByteArray data); // 将新收到的字节追加到缓存。
    void reset(); // 丢弃缓存中的未完成报文，关闭网络时调用。
    void analyze(); // 从缓存提取完整 Params{...} 报文，交给 parseFrame()。

signals:
    void update_buffer(); // 预留的缓存更新信号，当前未使用、未发出。
    // 六项数据全部有效时发出，参数依次对应温度、空气湿度、光照、土壤湿度、烟雾、雨量。
    void sensorDataParsed(
        double temp,
        double humi,
        double light,
        double soil,
        double mq2,
        double rain
    );
private:
    QByteArray m_buffer; // 保存尚未处理完的字节，支持分多次接收一条报文。

    void parseFrame(const QByteArray &body); // 检查括号内的字段和数值，合法时发出解析结果。
};

#endif // TCPPHRASER_H
