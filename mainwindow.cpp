#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_form.h"
#include "QTcpServer"
#include "QTcpSocket"
#include <QNetworkProxy>
#include <QtCharts/QChart>
#include <QtCharts/QValueAxis>
#include <QDateTime>
#include <QPainter>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_form(new Form)
    , m_server(new QTcpServer)
    , m_socket(new QTcpSocket)
    , m_socket_l(QVector<QTcpSocket*>())
    , m_tcpphraser(new TcpPhraser)
{
    // 1. 准备主界面、调试窗口控件和图表，填写默认 IP 与端口。
    Ui::Form* fui = m_form->get_ui();
    ui->setupUi(this);
    initCharts();
    ui->netlineEditip->setText("127.0.0.1");
    ui->netlineEdithost->setText("9494");
    m_socket->setProxy(QNetworkProxy::NoProxy);

    // 2. 模式按钮：选择客户端或服务器，网络由总开关启动。
    connect(ui->actionnetbtn, &QAction::triggered, this, [this]() {
        isServerMode = !isServerMode;
        ui->actionnetbtn->setIcon(QIcon(isServerMode ? ":/img/server.png" : ":/img/client.png"));
        ui->actionnetbtn->setToolTip(isServerMode ? "当前：服务器；点击切换为客户端"
                                                : "当前：客户端；点击切换为服务器");
    });
    ui->actionnetbtn->setToolTip("当前：客户端；点击切换为服务器");

    // 3. 客户端接收：读取消息，显示到调试窗口，再交给解析器。
    connect(m_socket, &QTcpSocket::readyRead, this, [this, fui]() {
        const QByteArray data = m_socket->readAll();
        fui->textBrowserrecv->append("收到:" + QString::fromUtf8(data));
        m_tcpphraser->append_data(data);
        m_tcpphraser->analyze();
    });

    // 4. 服务器接收：保存新接入的客户端，为它连接接收和断开事件。
    connect(m_server, &QTcpServer::newConnection, this, [this, fui]() {
        while (m_server->hasPendingConnections()) {
            QTcpSocket* socket = m_server->nextPendingConnection();
            m_socket_l.append(socket);
            // 这个客户端发来消息时，显示消息并解析传感器数值。
            connect(socket, &QTcpSocket::readyRead, this, [this, fui, socket]() {
                const QByteArray data = socket->readAll();
                fui->textBrowserrecv->append("收到:" + QString::fromUtf8(data));
                m_tcpphraser->append_data(data);
                m_tcpphraser->analyze();
            });
            // 客户端断开后，从列表移除，并释放它的通信对象。
            connect(socket, &QTcpSocket::disconnected, this, [this, socket]() {
                m_socket_l.removeOne(socket);
                socket->deleteLater();
            });
        }
    });

    // 5. 调试发送：网络开启且已经连接时，发送输入的消息。
    // 客户端模式发给服务器；服务器模式发给最后接入的客户端。
    connect(m_form, &Form::sendmsg, this, [this](QByteArray msg) {
        if (!ui->actionNetworkSwitch->isChecked())
            return;
        QTcpSocket* socket = isServerMode
                ? (m_socket_l.isEmpty() ? nullptr : m_socket_l.last()) : m_socket;
        if (socket && socket->state() == QAbstractSocket::ConnectedState)
            socket->write(msg);
    });

    // 6. LED 按钮：checked 为 true 发开灯指令，为 false 发关灯指令。
    // 灯泡图标表示按钮选择的状态，设备是否执行需要由设备端确认。
    connect(ui->actionLed, &QAction::triggered, this, [this](bool checked) {
        QTcpSocket* socket = isServerMode
                ? (m_socket_l.isEmpty() ? nullptr : m_socket_l.last()) : m_socket;
        if (socket)
            socket->write(checked ? "led_on" : "led_off");
        ui->actionLed->setIcon(QIcon(checked ? ":/img/led_on.png" : ":/img/led_off.png"));
        ui->actionLed->setToolTip(checked ? "点击发送关灯指令" : "点击发送开灯指令");
    });

    // 7. 网络总开关：开启时连接或监听，关闭时断开所有连接。
    connect(ui->actionNetworkSwitch, &QAction::toggled, this, [this](bool enabled) {
        if (enabled) {
            // 先读取端口；服务器开始监听，客户端主动连接。
            const quint16 port = ui->netlineEdithost->text().toUShort();
            if (isServerMode) {
                m_server->listen(QHostAddress(ui->netlineEditip->text()), port);
            } else {
                m_socket->connectToHost(ui->netlineEditip->text(), port);
            }
        } else {
            // 先停止监听，再断开连接，清空尚未解析完的消息。
            m_server->close();
            m_socket->abort();
            const auto sockets = m_socket_l;
            m_socket_l.clear();
            for (auto socket : sockets)
                socket->abort();
            m_tcpphraser->reset();
        }
        // 更新开关图标；联网期间锁定模式、IP 和端口，并启用 LED 按钮。
        ui->actionNetworkSwitch->setIcon(QIcon(enabled ? ":/img/open.png" : ":/img/close.png"));
        ui->actionNetworkSwitch->setToolTip(enabled ? "网络已开启；点击关闭" : "网络已关闭；点击开启");
        ui->actionnetbtn->setEnabled(!enabled);
        ui->actionLed->setEnabled(enabled);
        ui->netlineEditip->setEnabled(!enabled);
        ui->netlineEdithost->setEnabled(!enabled);
    });
    // 8. 调试按钮：显示收发消息的调试窗口。
    connect(ui->actiondebug, &QAction::triggered, this, [&](){
            m_form->show();
    });

    // 9. 解析完成：保存最新的六项数值供曲线使用，同时更新界面标签。
    connect(m_tcpphraser, &TcpPhraser::sensorDataParsed,
            this, [this](double temp,
                         double humi,
                         double light,
                         double soil,
                         double mq2,
                         double rain)
    {
        m_sensorData = {temp, humi, light, soil, mq2, rain};
        m_hasSensorData = true;
        ui->temperatureValueLabel->setText(QString::number(temp));
        ui->humidityValueLabel->setText(QString::number(humi));
        ui->lightValueLabel->setText(QString::number(light));
        ui->soilValueLabel->setText(QString::number(soil));
        ui->smokeValueLabel->setText(QString::number(mq2));
        ui->rainValueLabel->setText(QString::number(rain));
    });


}

MainWindow::~MainWindow()
{
    delete ui;
}

// 创建两个图表，每个图表放三条曲线，再用定时器每秒添加数据点。
void MainWindow::initCharts()
{
    // 两个图表共用同一套创建步骤；曲线名称与传感器数值的顺序对应。
    QtCharts::QChartView* views[] = {ui->graphicsView, ui->graphicsView_2};
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

    // 定时器每秒执行一次；收到第一组传感器数据后才开始画线。
    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        if (!m_hasSensorData)
            return;

        const QDateTime now = QDateTime::currentDateTime();
        // 第一笔数据作为起点，先从左往右画满一分钟。
        if (m_series[0]->count() == 0) {
            for (auto axis : m_timeAxes)
                axis->setRange(now, now.addSecs(60));
        }
        // Qt 时间轴使用毫秒；cutoff 是一分钟前的时间，用来清理旧点。
        const qint64 cutoff = now.addSecs(-60).toMSecsSinceEpoch();
        const double values[] = {m_sensorData.temperature, m_sensorData.humidity,
                                 m_sensorData.light, m_sensorData.soil,
                                 m_sensorData.smoke, m_sensorData.rain};


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


    });
    timer->start(1000);
}

