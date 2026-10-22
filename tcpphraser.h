#ifndef TCPPHRASER_H
#define TCPPHRASER_H

#include <QObject>
#include "QString"
#include "QByteArray"

class TcpPhraser : public QObject
{
    Q_OBJECT
public:
    explicit TcpPhraser(QObject *parent = nullptr);
    void append_data(const QByteArray data);
    void reset();
    void analyze();

signals:
    void update_buffer();
    void sensorDataParsed(
        double temp,
        double humi,
        double light,
        double soil,
        double mq2,
        double rain
    );
private:
    QByteArray m_buffer;

    void parseFrame(const QByteArray &body);
};

#endif // TCPPHRASER_H
