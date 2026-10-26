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
    , m_databasemanger(new DataBaseManger)
    , m_thread(new QThread(this))

{
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


    m_history->setWindowFlag(Qt::Window);

    connect(ui->actionHistory, &QAction::triggered, this, [this]() {
        m_history->show();
        m_history->raise();
        m_history->activateWindow();
    });


    ui->netlineEditip->setText("127.0.0.1");
    ui->netlineEdithost->setText("9494");

    // 模式按钮只选择角色，网络由总开关启动。
    connect(ui->actionnetbtn, &QAction::triggered, this, [this]() {
        isServerMode = !isServerMode;
        ui->actionnetbtn->setIcon(QIcon(isServerMode ? ":/img/server.png" : ":/img/client.png"));
        ui->actionnetbtn->setToolTip(isServerMode ? "当前：服务器；点击切换为客户端"
                                                : "当前：客户端；点击切换为服务器");
    });
    ui->actionnetbtn->setToolTip("当前：客户端；点击切换为服务器");

    // 原始消息都显示在调试窗口，只有解析有效的报文才更新传感器和数据库。
    connect(m_tcpmanger, &Tcpmanger::dataReceived, this, [this, fui](QByteArray data) {
        fui->textBrowserrecv->append("收到:" + QString::fromUtf8(data));
        m_tcpphraser->append_data(data);
        m_tcpphraser->analyze();
    });

    m_form->setWindowFlag(Qt::Window);
    connect(m_form, &Form::sendmsg, m_tcpmanger, &Tcpmanger::send);
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

    // 总开关控制网络启停；关闭时同时清空未解析完的报文。
    connect(ui->actionNetworkSwitch, &QAction::toggled, this, [this](bool enabled) {
        if (enabled) {
            const quint16 port = ui->netlineEdithost->text().toUShort();
            m_tcpmanger->start(isServerMode, ui->netlineEditip->text(), port);
        } else {
            m_tcpmanger->stop();
            m_tcpphraser->reset();
        }
        // 开关开启期间锁定连接参数；开关状态不等于连接成功。
        ui->actionNetworkSwitch->setIcon(QIcon(enabled ? ":/img/open.png" : ":/img/close.png"));
        ui->actionNetworkSwitch->setToolTip(enabled ? "网络已开启；点击关闭" : "网络已关闭；点击开启");
        ui->actionnetbtn->setEnabled(!enabled);
        ui->actionLed->setEnabled(enabled);
        ui->setLightButton->setEnabled(enabled);
        ui->netlineEditip->setEnabled(!enabled);
        ui->netlineEdithost->setEnabled(!enabled);
    });
    connect(ui->actiondebug, &QAction::triggered, this, [&](){
            m_form->show();
    });

    // 有效数据立即更新标签，入库交给数据库线程，曲线由定时器刷新。
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
    m_tcpmanger->stop();
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
