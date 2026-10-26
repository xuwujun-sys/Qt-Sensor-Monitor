#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "form.h"
#include "tcpmanger.h"
#include "tcpphraser.h"
#include "sensordata.h"
#include "sensorchart.h"
#include "databasemanger.h"
#include "historywidget.h"

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
    HistoryWidget* m_history;

    // 网络通信
    bool isServerMode = false;  // false：客户端模式；true：服务器模式
    Tcpmanger* m_tcpmanger;     // 负责连接、监听、收发和关闭网络。

    // 传感器数据
    TcpPhraser* m_tcpphraser;    // 把收到的消息解析成六项传感器数值
    SensorData m_sensorData{};  // 保存最近一次收到的六项数值
    bool m_hasSensorData = false; // 收到第一组数据后为 true，曲线才开始画

    // 实时曲线：负责两个图表，定时刷新仍由主窗口安排。
    SensorChart* m_sensorchart;

    DataBaseManger* m_databasemanger;
    void refreshHistory(); // 查询数据库并刷新历史表格。
};
#endif // MAINWINDOW_H
