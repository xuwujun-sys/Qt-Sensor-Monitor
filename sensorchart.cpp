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
    // 两个图表共用同一套创建步骤；曲线名称与传感器数值的顺序对应。
    QtCharts::QChartView* views[] = {view1, view2};
    const QString names[] = {QStringLiteral("温度"), QStringLiteral("空气湿度"),
                             QStringLiteral("光照"), QStringLiteral("土壤湿度"),
                             QStringLiteral("烟雾"), QStringLiteral("雨量")};

    const QDateTime now = QDateTime::currentDateTime();

    for (int chartIndex = 0; chartIndex < 2; ++chartIndex) {
        // 创建图表并放入界面，开启抗锯齿让线条更平滑。
        auto chart = new QtCharts::QChart;
        views[chartIndex]->setChart(chart);
        views[chartIndex]->setRenderHint(QPainter::Antialiasing);

        // 横坐标：显示“分:秒”，三个刻度相隔 30 秒。
        auto axisX = new QtCharts::QDateTimeAxis;
        axisX->setFormat("mm:ss");
        axisX->setTickCount(3);
        axisX->setRange(now, now.addSecs(60));
        chart->addAxis(axisX, Qt::AlignBottom);
        m_timeAxes[chartIndex] = axisX;

        // 纵坐标：固定显示 0～100，分成五个刻度。
        auto axisY = new QtCharts::QValueAxis;
        axisY->setRange(0, 100);
        axisY->setTickCount(5);
        chart->addAxis(axisY, Qt::AlignLeft);

        // 每张图创建三条曲线，并让它们使用这张图的横、纵坐标。
        for (int j = 0; j < 3; ++j) {
            // 第一张图对应 0、1、2，第二张图对应 3、4、5。
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

// 使用传入的数值更新曲线，沿用原来的一分钟滚动逻辑。
void SensorChart::appendData(const SensorData &data)
{
    const QDateTime now = QDateTime::currentDateTime();
    // 第一笔数据作为起点，先从左往右画满一分钟。
    if (m_series[0]->count() == 0) {
        for (auto axis : m_timeAxes)
            axis->setRange(now, now.addSecs(60));
    }
    // Qt 时间轴使用毫秒；cutoff 是一分钟前的时间，用来清理旧点。
    const qint64 cutoff = now.addSecs(-60).toMSecsSinceEpoch();
    const double values[] = {data.temperature, data.humidity,
                             data.light, data.soil,
                             data.smoke, data.rain};


    for (int i = 0; i < 6; ++i) {
        // 一个点由“当前时间”和“最新数值”组成，相邻点会自动连线。
        m_series[i]->append(now.toMSecsSinceEpoch(), values[i]);
        // 只保留最近一分钟，避免运行时间越长占用越多内存。
        while (m_series[i]->count() > 0 && m_series[i]->at(0).x() < cutoff)
            m_series[i]->remove(0);
    }
    // 画满一分钟后，横坐标随时间滚动，始终显示最近一分钟。
    for (auto axis : m_timeAxes) {
        if (now > axis->max())
            axis->setRange(now.addSecs(-60), now);
    }
}
