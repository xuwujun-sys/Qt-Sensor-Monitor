#ifndef SENSORDATA_H
#define SENSORDATA_H

#include <QMetaType>

// 一组传感器数值，供界面显示、曲线绘制和数据库存储共用。
struct SensorData
{
    double temperature; // 温度，对应协议 temp。
    double humidity;    // 空气湿度，对应协议 humi。
    double light;       // 光照，对应协议 light。
    double soil;        // 土壤湿度，对应协议 soil。
    double smoke;       // 烟雾数值，对应协议 mq2。
    double rain;        // 雨滴数值，对应协议 rain。
};

Q_DECLARE_METATYPE(SensorData) // 允许 SensorData 通过跨线程信号传递。


#endif // SENSORDATA_H
