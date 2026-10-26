#ifndef SENSORCHART_H
#define SENSORCHART_H

#include <QObject>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QDateTimeAxis>
#include "sensordata.h"

// 管理两张图共六条曲线，刷新频率由调用方决定。
class SensorChart : public QObject
{
    Q_OBJECT

public:
    explicit SensorChart(QObject *parent = nullptr); // 创建管理对象，图表稍后初始化。

    // 使用有效的图表控件初始化一次，再调用 appendData()。
    void initCharts(QtCharts::QChartView *view1, QtCharts::QChartView *view2);
    void appendData(const SensorData &data); // 为六条曲线加点，并清理旧点、滚动时间轴。

private:
    // 顺序：温度、空气湿度、光照、土壤湿度、烟雾、雨量。
    QtCharts::QLineSeries* m_series[6]{}; // 六条传感器曲线，每张图三条。
    QtCharts::QDateTimeAxis* m_timeAxes[2]{}; // 两张图的时间横轴，显示一分钟范围。
    // 坐标轴和曲线由图表拥有，本类不手动删除。
};

#endif // SENSORCHART_H
