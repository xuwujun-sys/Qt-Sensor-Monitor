#include "mainwindow.h"
#include "historywidget.h"
#include "ui_mainwindow.h"
#include "ui_form.h"
#include <QDateTime>
#include <QTimer>
#include <QSignalBlocker>
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
    , m_serialmanger(new SerialManger(this))
    , m_tcpphraser(new TcpPhraser(this))
    , m_sensorchart(new SensorChart(this))
    , m_databasemanger(new DataBaseManger)
    , m_thread(new QThread(this))

{
    // 1. 创建界面并初始化图表；fui 用于向独立调试窗口追加接收内容。
    Ui::Form* fui = m_form->get_ui();
    ui->setupUi(this);
    m_sensorchart->initCharts(ui->graphicsView, ui->graphicsView_2);
    // 曲线每秒取最新值；没有新数据时，继续绘制上一次有效值。
    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        if (m_hasSensorData)
            m_sensorchart->appendData(m_sensorData);
    });
    timer->start(1000);
    // 2. 数据库在工作线程执行；跨线程信号将保存、查询请求排队处理。
    // 注册跨线程传递的数据类型，再把数据库对象移入工作线程。
    qRegisterMetaType<SensorData>("SensorData");
    qRegisterMetaType<QList<QVariantMap>>("QList<QVariantMap>");
    m_databasemanger->moveToThread(m_thread);
    connect(m_thread, &QThread::started, m_databasemanger, &DataBaseManger::initialize);
    connect(m_thread, &QThread::finished, m_databasemanger, &QObject::deleteLater);
    connect(this, &MainWindow::saveRequested, m_databasemanger, &DataBaseManger::saveData);
    connect(this, &MainWindow::queryRequested, m_databasemanger, &DataBaseManger::queryData);
    connect(m_databasemanger, &DataBaseManger::dataReady, this, &MainWindow::refreshHistory);
    connect(m_history, &HistoryWidget::refreshRequested,
            this, &MainWindow::queryRequested);
    m_thread->start();


    // 3. 历史窗口作为独立窗口显示；刷新请求经主窗口转交数据库线程。
    m_history->setWindowFlag(Qt::Window);

    connect(ui->actionHistory, &QAction::triggered, this, [this]() {
        m_history->show();
        m_history->raise();
        m_history->activateWindow();
    });


    // 4. 设置默认 TCP 地址，扫描一次串口；之后通过“刷新”按钮重新扫描。
    ui->netlineEditip->setText("127.0.0.1");
    ui->netlineEdithost->setText("9494");

    refreshSerialPorts();
    connect(ui->refreshSerialPortsButton, &QPushButton::clicked,
            this, &MainWindow::refreshSerialPorts);
    connect(ui->communicationComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
        // 下拉框顺序：0 为 TCP，1 为串口；页面切换已在 .ui 中连接。
        // 此处只更新工具栏，切换通信方式本身不会打开连接。
        ui->actionnetbtn->setEnabled(index == 0);
        ui->actionNetworkSwitch->setText(index == 0 ? "网络开关" : "串口开关");
        ui->actionNetworkSwitch->setToolTip("点击开启当前通信方式");
    });

    // 模式按钮只选择角色，网络由总开关启动。
    connect(ui->actionnetbtn, &QAction::triggered, this, [this]() {
        isServerMode = !isServerMode;
        ui->actionnetbtn->setIcon(QIcon(isServerMode ? ":/img/server.png" : ":/img/client.png"));
        ui->actionnetbtn->setToolTip(isServerMode ? "当前：服务器；点击切换为客户端"
                                                : "当前：客户端；点击切换为服务器");
    });
    ui->actionnetbtn->setToolTip("当前：客户端；点击切换为服务器");

    // 5. TCP 接收：先显示原始文本，再缓存、提取完整报文；允许一条报文分多次到达。
    // 只有解析有效的报文才更新传感器和数据库。
    connect(m_tcpmanger, &Tcpmanger::dataReceived, this, [this, fui](QByteArray data) {
        fui->textBrowserrecv->append("收到:" + QString::fromUtf8(data));
        m_tcpphraser->append_data(data);
        m_tcpphraser->analyze();
    });

    m_form->setWindowFlag(Qt::Window);
    // 调试发送统一交给主窗口选择通道，Form 不直接依赖 TCP 或串口类。
    connect(m_form, &Form::sendmsg, this, &MainWindow::sendDebugData);
    // 串口接收目前只作 UTF-8 文本显示，不进入传感器解析、绘图和入库流程。
    connect(m_serialmanger, &SerialManger::dataReceived, this, [fui](const QByteArray &data) {
        fui->textBrowserrecv->append("收到:" + QString::fromUtf8(data));
    });
    // 6. 设备控制：LED 和 PWM 指令目前通过 TCP 下发，串口模式下禁用对应控件。
    // 图标表示下发的开关状态，不代表设备已确认执行。
    connect(ui->actionLed, &QAction::triggered, this, [this](bool checked) {
        m_tcpmanger->send(checked ? "led_on" : "led_off");
        ui->actionLed->setIcon(QIcon(checked ? ":/img/led_on.png" : ":/img/led_off.png"));
        ui->actionLed->setToolTip(checked ? "点击发送关灯指令" : "点击发送开灯指令");
    });

    // 拖动只更新显示，点击“设定”才下发光强。
    connect(ui->lightSlider, &QSlider::valueChanged, this, [this](int value) {
        ui->lightPwmValueLabel->setText(QString::number(value) + "%");
    });
    connect(ui->setLightButton, &QPushButton::clicked, this, [this]() {
        const QString command = "Pwm:" + QString::number(ui->lightSlider->value());
        m_tcpmanger->send(command.toUtf8());
    });

    // 7. 顶部总开关统一进入 toggleCommunication；调试按钮仅负责显示调试窗口。
    connect(ui->actionNetworkSwitch, &QAction::toggled,
            this, &MainWindow::toggleCommunication);
    connect(ui->actiondebug, &QAction::triggered, this, [&](){
            m_form->show();
    });

    // 8. 解析成功后的数据分流：更新最新值和标签，发出保存请求。
    // 入库按有效报文触发；绘图按每秒一次的定时器触发，两者频率互不绑定。
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

        emit saveRequested(m_sensorData);
    });


}

MainWindow::~MainWindow()
{
    // 先停止通信，再等待数据库线程退出，最后释放主窗口 UI。
    m_tcpmanger->stop();
    m_serialmanger->close();
    // 将退出排在已提交的数据库任务后面，避免尚未保存就结束线程。
    QMetaObject::invokeMethod(m_databasemanger, [this]() {
        m_thread->quit();
    }, Qt::QueuedConnection);
    m_thread->wait();
    delete ui;
}

// 查询在工作线程完成，这里只在主线程更新模型。
void MainWindow::refreshHistory(const QList<QVariantMap> &rows)
{
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

// 启动和点击“刷新”时调用；每次重建列表，选择项恢复为扫描结果的第一项。
void MainWindow::refreshSerialPorts()
{
    ui->serialPortComboBox->clear();
    ui->serialPortComboBox->addItems(m_serialmanger->portNames());

    ui->serialHintLabel->setText(
        ui->serialPortComboBox->count()
            ? "已发现串口，请选择串口"
            : "未发现串口，请连接后刷新"
    );
}

// 调试窗口已经把输入转成 UTF-8；这里原样转发，不额外追加换行或协议字段。
// 通道未打开时，底层 send() 会忽略发送请求。
void MainWindow::sendDebugData(const QByteArray &data)
{
    if (ui->communicationComboBox->currentIndex() == 0)
        m_tcpmanger->send(data);
    else
        m_serialmanger->send(data);
}

// 顶部开关的槽函数：enabled=true 请求打开，false 请求关闭当前通信方式。
// 原来的 TCP 总开关逻辑也集中在这里，避免构造函数堆积启停细节。
void MainWindow::toggleCommunication(bool enabled)
{
    const bool serialMode = ui->communicationComboBox->currentIndex() == 1;
    if (serialMode) {
        // 串口 open() 同步返回结果；固定 8N1、无流控由 SerialManger 内部配置。
        if (enabled && !m_serialmanger->open(ui->serialPortComboBox->currentText(),
                                            ui->serialBaudRateComboBox->currentText().toInt())) {
            // 打开失败时恢复开关；临时屏蔽 toggled，避免恢复操作再次调用本函数。
            const QSignalBlocker blocker(ui->actionNetworkSwitch);
            ui->actionNetworkSwitch->setChecked(false);
            ui->serialHintLabel->setText("打开失败，请检查端口或是否被占用");
            return;
        }
        if (!enabled)
            m_serialmanger->close();
        ui->serialHintLabel->setText(enabled ? "串口已打开" : "串口已关闭");
    } else if (enabled) {
        // TCP 启动角色由模式按钮决定；客户端连接过程由 Qt 异步完成。
        const quint16 port = ui->netlineEdithost->text().toUShort();
        m_tcpmanger->start(isServerMode, ui->netlineEditip->text(), port);
    } else {
        m_tcpmanger->stop();
        // 丢弃上次连接残留的半条报文，避免与下一次连接的数据拼接。
        m_tcpphraser->reset();
    }
    // 开关开启期间锁定连接参数；开关状态不等于连接成功。
    ui->actionNetworkSwitch->setIcon(QIcon(enabled ? ":/img/open.png" : ":/img/close.png"));
    ui->actionNetworkSwitch->setToolTip(enabled ? "点击关闭当前通信方式" : "点击开启当前通信方式");
    // TCP 角色仅在关闭且选择 TCP 时可切换；LED 和光强控制仅在 TCP 开启时可用。
    ui->actionnetbtn->setEnabled(!enabled && !serialMode);
    ui->actionLed->setEnabled(enabled && !serialMode);
    ui->setLightButton->setEnabled(enabled && !serialMode);
    // 开启期间锁定两种方式的参数及通信选择，关闭后允许重新配置。
    ui->netlineEditip->setEnabled(!enabled);
    ui->netlineEdithost->setEnabled(!enabled);
    ui->communicationComboBox->setEnabled(!enabled);
    ui->serialPortComboBox->setEnabled(!enabled);
    ui->serialBaudRateComboBox->setEnabled(!enabled);
    ui->refreshSerialPortsButton->setEnabled(!enabled);
}
