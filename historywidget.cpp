#include "historywidget.h"
#include "ui_historywidget.h"
#include <QHeaderView>
#include <QStandardItemModel>

HistoryWidget::HistoryWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::HistoryWidget)
    , m_model(new QStandardItemModel(0, 7, this))
{
    ui->setupUi(this);

    // 模型只创建一次，刷新时保留表头并替换数据行。
    m_model->setHorizontalHeaderLabels({QStringLiteral("时间"), QStringLiteral("温度"),
        QStringLiteral("空气湿度"), QStringLiteral("光照"), QStringLiteral("土壤湿度"),
        QStringLiteral("烟雾"), QStringLiteral("雨量")});
    ui->tableView->setModel(m_model);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
    ui->tableView->setColumnWidth(0, 210);

    // 窗口只发出操作请求，不直接访问数据库；主窗口负责连接请求与处理逻辑。
    // clearRequested 目前尚未在 MainWindow 接入，因此“清空”按钮没有实际清除行为。
    connect(ui->refreshButton, &QPushButton::clicked, this, &HistoryWidget::refreshRequested);
    connect(ui->clearButton, &QPushButton::clicked, this, &HistoryWidget::clearRequested);
    connect(ui->closeButton, &QPushButton::clicked, this, &QWidget::close);
}

HistoryWidget::~HistoryWidget()
{
    delete ui;
}

QStandardItemModel* HistoryWidget::get_model() const
{
    return m_model;
}
