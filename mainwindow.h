#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "form.h"
#include "QTcpServer"
#include "QTcpSocket"
#include "tcpphraser.h"
#include "sensordata.h"
#include <QtCharts/QLineSeries>
#include <QtCharts/QDateTimeAxis>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    // 窗口
    Ui::MainWindow *ui;          // 主窗口里的按钮、标签、图表等控件
    Form* m_form;               // 收发消息用的调试窗口

    // 网络通信
    bool isServerMode = false;  // false：客户端模式；true：服务器模式
    QTcpServer* m_server;       // 服务器：监听端口，接收客户端连接
    QTcpSocket* m_socket;       // 客户端：连接服务器，收发数据
    QVector<QTcpSocket*> m_socket_l; // 服务器模式下，保存已接入的客户端

    // 传感器数据
    TcpPhraser* m_tcpphraser;    // 把收到的消息解析成六项传感器数值
    SensorData m_sensorData{};  // 保存最近一次收到的六项数值
    bool m_hasSensorData = false; // 收到第一组数据后为 true，曲线才开始画

    // 实时曲线
    // 顺序：温度、空气湿度、光照、土壤湿度、烟雾、雨量
    QtCharts::QLineSeries* m_series[6]{};
    QtCharts::QDateTimeAxis* m_timeAxes[2]{}; // 两个图表各自的时间横坐标
    void initCharts();          // 创建曲线和坐标轴，设置每秒刷新
};
#endif // MAINWINDOW_H
