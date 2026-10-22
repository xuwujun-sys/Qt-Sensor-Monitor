#include "historywidget.h"
#include "ui_historywidget.h"
#include <QHeaderView>
#include <QStandardItemModel>

HistoryWidget::HistoryWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::HistoryWidget)
{
    ui->setupUi(this);

    // 先放好表头，后续接入数据库模型后显示真实数据。
    auto model = new QStandardItemModel(0, 7, this);
    model->setHorizontalHeaderLabels({QStringLiteral("时间"), QStringLiteral("温度"),
        QStringLiteral("空气湿度"), QStringLiteral("光照"), QStringLiteral("土壤湿度"),
        QStringLiteral("烟雾"), QStringLiteral("雨量")});
    ui->tableView->setModel(model);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
    ui->tableView->setColumnWidth(0, 210);

    connect(ui->refreshButton, &QPushButton::clicked, this, &HistoryWidget::refreshRequested);
    connect(ui->clearButton, &QPushButton::clicked, this, &HistoryWidget::clearRequested);
    connect(ui->closeButton, &QPushButton::clicked, this, &QWidget::close);
}

HistoryWidget::~HistoryWidget()
{
    delete ui;
}
