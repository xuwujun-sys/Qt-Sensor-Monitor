#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include "form.h"
#include "tcpmanger.h"
#include "serialmanger.h"
#include "tcpphraser.h"
#include "sensordata.h"
#include "sensorchart.h"
#include "databasemanger.h"
#include "historywidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// 连接界面、通信、解析、曲线和数据库，协调数据流转。
// TCP 接收 -> 调试显示、协议解析 -> 数值标签、数据库；定时器用最新数值绘图。
// 串口接收 -> 调试显示；调试发送 -> 根据通信下拉框选择 TCP 或串口。
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr); // 创建界面和各功能对象，连接信号与槽。
    ~MainWindow(); // 释放 UI；以主窗口为父对象的功能对象由 Qt 自动释放。

signals:
    void saveRequested(SensorData data); // 向数据库线程提交保存任务。
    void queryRequested(); // 向数据库线程提交查询任务。

private:
    // 窗口
    Ui::MainWindow *ui;       // 访问主窗口的按钮、标签、输入框等控件。
    Form* m_form;             // 显示原始收发消息的调试窗口。
    HistoryWidget* m_history; // 显示数据库记录的历史窗口。

    // 通信管理：底层类负责收发，主窗口负责读取配置和更新控件状态。
    bool isServerMode = false; // false 为客户端，true 为服务器，不代表连接状态。
    Tcpmanger* m_tcpmanger;    // 执行网络启停、接收与发送。
    SerialManger* m_serialmanger; // 管理串口，使用界面选择的端口和波特率。
    void refreshSerialPorts(); // 刷新串口列表。
    void toggleCommunication(bool enabled); // 按所选方式打开或关闭通信。
    void sendDebugData(const QByteArray &data); // 调试消息发送到当前通信方式。

    // 传感器数据
    TcpPhraser* m_tcpphraser;   // 缓存收到的字节，解析有效的传感器报文。
    SensorData m_sensorData{};  // 最近一次有效数据，供定时绘图使用。
    bool m_hasSensorData = false; // 收到过有效数据后开始绘图，断网时不清除此标志。

    // 绘图与数据存储
    SensorChart* m_sensorchart; // 创建并更新两张实时曲线图。

    DataBaseManger* m_databasemanger; // 无父对象，在线程结束时自动释放。
    QThread* m_thread; // 数据库工作线程，退出前等待任务处理完毕。
    void refreshHistory(const QList<QVariantMap> &rows); // 在主线程将查询结果填入表格。
};
#endif // MAINWINDOW_H
