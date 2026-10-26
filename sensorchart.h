#ifndef SENSORCHART_H
#define SENSORCHART_H

#include <QObject>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QDateTimeAxis>
#include "sensordata.h"

class SensorChart : public QObject
{
    Q_OBJECT

public:
    explicit SensorChart(QObject *parent = nullptr);

    // 启动时调用一次，使用界面中现有的两个图表控件。
    void initCharts(QtCharts::QChartView *view1, QtCharts::QChartView *view2);
    // 主窗口每秒传入最新数据，由本类添加点、清理旧点和滚动时间轴。
    void appendData(const SensorData &data);

private:
    // 顺序：温度、空气湿度、光照、土壤湿度、烟雾、雨量。
    QtCharts::QLineSeries* m_series[6]{};
    QtCharts::QDateTimeAxis* m_timeAxes[2]{};
};

#endif // SENSORCHART_H
