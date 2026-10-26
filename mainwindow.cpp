#include "mainwindow.h"
#include "historywidget.h"
#include "ui_mainwindow.h"
#include "ui_form.h"
#include <QDateTime>
#include <QTimer>
#include "historywidget.h"
#include "ui_historywidget.h"
#include <QHeaderView>
#include <QStandardItemModel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_form(new Form(this))
    , m_history(new HistoryWidget(this))
    , m_tcpmanger(new Tcpmanger(this))
    , m_tcpphraser(new TcpPhraser(this))
    , m_sensorchart(new SensorChart(this))
    , m_databasemanger(new DataBaseManger(this))

{
    // 1. 准备主界面、调试窗口控件和图表，填写默认 IP 与端口。
    Ui::Form* fui = m_form->get_ui();
    ui->setupUi(this);
    m_sensorchart->initCharts(ui->graphicsView, ui->graphicsView_2);
    // 主窗口决定何时更新；曲线类只负责绘图。
    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        if (m_hasSensorData)
            m_sensorchart->appendData(m_sensorData);
    });
    timer->start(1000);
    m_databasemanger->open(); // 启动时打开数据库，退出时由析构函数关闭。
    connect(m_history, &HistoryWidget::refreshRequested,
            this, &MainWindow::refreshHistory);
    refreshHistory();


    // 历史数据窗口：通过工具栏的数据库图标打开。
    m_history->setWindowFlag(Qt::Window);

    connect(ui->actionHistory, &QAction::triggered, this, [this]() {
        m_history->show();
        m_history->raise();
        m_history->activateWindow();
    });


    ui->netlineEditip->setText("127.0.0.1");
    ui->netlineEdithost->setText("9494");

    // 2. 模式按钮：选择客户端或服务器，网络由总开关启动。
    connect(ui->actionnetbtn, &QAction::triggered, this, [this]() {
        isServerMode = !isServerMode;
        ui->actionnetbtn->setIcon(QIcon(isServerMode ? ":/img/server.png" : ":/img/client.png"));
        ui->actionnetbtn->setToolTip(isServerMode ? "当前：服务器；点击切换为客户端"
                                                : "当前：客户端；点击切换为服务器");
    });
    ui->actionnetbtn->setToolTip("当前：客户端；点击切换为服务器");

    // 3. TCP 收到数据后，主窗口显示消息并交给解析器。
    connect(m_tcpmanger, &Tcpmanger::dataReceived, this, [this, fui](QByteArray data) {
        fui->textBrowserrecv->append("收到:" + QString::fromUtf8(data));
        m_tcpphraser->append_data(data);
        m_tcpphraser->analyze();
    });

    // 4. 调试窗口只负责提供消息，发送目标由 TCP 类选择。
    m_form->setWindowFlag(Qt::Window);
    connect(m_form, &Form::sendmsg, m_tcpmanger, &Tcpmanger::send);
    // 5. LED 按钮：checked 为 true 发开灯指令，为 false 发关灯指令。
    // 灯泡图标表示按钮选择的状态，设备是否执行需要由设备端确认。
    connect(ui->actionLed, &QAction::triggered, this, [this](bool checked) {
        m_tcpmanger->send(checked ? "led_on" : "led_off");
        ui->actionLed->setIcon(QIcon(checked ? ":/img/led_on.png" : ":/img/led_off.png"));
        ui->actionLed->setToolTip(checked ? "点击发送关灯指令" : "点击发送开灯指令");
    });

    // 光强控制：拖动时显示百分比，点击“设定”才发送指令。
    connect(ui->lightSlider, &QSlider::valueChanged, this, [this](int value) {
        ui->lightPwmValueLabel->setText(QString::number(value) + "%");
    });
    connect(ui->setLightButton, &QPushButton::clicked, this, [this]() {
        const QString command = "Pwm:" + QString::number(ui->lightSlider->value());
        m_tcpmanger->send(command.toUtf8());
    });

    // 6. 网络总开关：开启时连接或监听，关闭时断开所有连接。
    connect(ui->actionNetworkSwitch, &QAction::toggled, this, [this](bool enabled) {
        if (enabled) {
            // 先读取端口；服务器开始监听，客户端主动连接。
            const quint16 port = ui->netlineEdithost->text().toUShort();
            m_tcpmanger->start(isServerMode, ui->netlineEditip->text(), port);
        } else {
            m_tcpmanger->stop();
            m_tcpphraser->reset();
        }
        // 更新开关图标；联网期间锁定模式、IP 和端口，并启用 LED 按钮。
        ui->actionNetworkSwitch->setIcon(QIcon(enabled ? ":/img/open.png" : ":/img/close.png"));
        ui->actionNetworkSwitch->setToolTip(enabled ? "网络已开启；点击关闭" : "网络已关闭；点击开启");
        ui->actionnetbtn->setEnabled(!enabled);
        ui->actionLed->setEnabled(enabled);
        ui->setLightButton->setEnabled(enabled);
        ui->netlineEditip->setEnabled(!enabled);
        ui->netlineEdithost->setEnabled(!enabled);
    });
    // 7. 调试按钮：显示收发消息的调试窗口。
    connect(ui->actiondebug, &QAction::triggered, this, [&](){
            m_form->show();
    });

    // 8. 解析完成：保存最新的六项数值供曲线使用，同时更新界面标签。
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

        if (m_databasemanger->insert(m_sensorData))
            refreshHistory();
    });


}

MainWindow::~MainWindow()
{
    delete ui;
}

// 查询全部记录，替换表格中的旧数据。
void MainWindow::refreshHistory()
{
    QList<QVariantMap> rows;
    if (!m_databasemanger->select(rows))
        return;

    auto model = m_history->get_model();
    model->setRowCount(0); // 清空旧数据，保留表头。
    for (const auto &row : rows) {
        model->appendRow({
            new QStandardItem(row["collect_time"].toDateTime().toString("yyyy-MM-dd hh:mm:ss")),
            new QStandardItem(row["temperature"].toString()),
            new QStandardItem(row["humidity"].toString()),
            new QStandardItem(row["light"].toString()),
            new QStandardItem(row["soil"].toString()),
            new QStandardItem(row["smoke"].toString()),
            new QStandardItem(row["rain"].toString())
        });
    }
}
