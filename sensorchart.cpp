#include "sensorchart.h"
#include <QtCharts/QChart>
#include <QtCharts/QValueAxis>
#include <QDateTime>
#include <QPainter>

SensorChart::SensorChart(QObject *parent) : QObject(parent)
{
}

void SensorChart::initCharts(QtCharts::QChartView *view1, QtCharts::QChartView *view2)
{
    // 两张图各显示三项数据，数组顺序与 SensorData 到 values[] 的映射保持一致。
    QtCharts::QChartView* views[] = {view1, view2};
    const QString names[] = {QStringLiteral("温度"), QStringLiteral("空气湿度"),
                             QStringLiteral("光照"), QStringLiteral("土壤湿度"),
                             QStringLiteral("烟雾"), QStringLiteral("雨量")};

    const QDateTime now = QDateTime::currentDateTime();

    for (int chartIndex = 0; chartIndex < 2; ++chartIndex) {
        auto chart = new QtCharts::QChart;
        views[chartIndex]->setChart(chart);
        views[chartIndex]->setRenderHint(QPainter::Antialiasing);

        // 一分钟窗口，时间标签显示“分:秒”。
        auto axisX = new QtCharts::QDateTimeAxis;
        axisX->setFormat("mm:ss");
        axisX->setTickCount(3);
        axisX->setRange(now, now.addSecs(60));
        chart->addAxis(axisX, Qt::AlignBottom);
        m_timeAxes[chartIndex] = axisX;

        // 同一张图的三条线共用纵轴，当前固定显示 0~100，不自动按数据缩放。
        auto axisY = new QtCharts::QValueAxis;
        axisY->setRange(0, 100);
        axisY->setTickCount(5);
        chart->addAxis(axisY, Qt::AlignLeft);

        for (int j = 0; j < 3; ++j) {
            // 前三项放第一张图，后三项放第二张图。
            const int index = chartIndex * 3 + j;
            auto series = new QtCharts::QLineSeries;
            series->setName(names[index]);
            chart->addSeries(series);
            series->attachAxis(axisX);
            series->attachAxis(axisY);
            m_series[index] = series;
        }
    }
}

void SensorChart::appendData(const SensorData &data)
{
    // 主窗口每秒调用一次；时间戳取绘图时刻，而非报文到达或数据库采集时刻。
    const QDateTime now = QDateTime::currentDateTime();
    // 第一笔数据作为起点，先从左往右画满一分钟。
    if (m_series[0]->count() == 0) {
        for (auto axis : m_timeAxes)
            axis->setRange(now, now.addSecs(60));
    }
    // 时间轴使用毫秒，只保留最近一分钟的数据点。
    const qint64 cutoff = now.addSecs(-60).toMSecsSinceEpoch();
    const double values[] = {data.temperature, data.humidity,
                             data.light, data.soil,
                             data.smoke, data.rain};


    for (int i = 0; i < 6; ++i) {
        m_series[i]->append(now.toMSecsSinceEpoch(), values[i]);
        while (m_series[i]->count() > 0 && m_series[i]->at(0).x() < cutoff)
            m_series[i]->remove(0);
    }
    // 画满一分钟后滚动，否则保持起点不动。
    for (auto axis : m_timeAxes) {
        if (now > axis->max())
            axis->setRange(now.addSecs(-60), now);
    }
}
