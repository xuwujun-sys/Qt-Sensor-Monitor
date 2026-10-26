#include "form.h"
#include "ui_form.h"

Form::Form(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form)
{
    // send、exit 按钮通过 setupUi 的自动连接调用对应 on_控件名_clicked 槽。
    ui->setupUi(this);
}

Ui::Form* Form::get_ui()
{
    return this->ui;
}

Form::~Form()
{
    delete ui;
}

void Form::on_exit_clicked()
{
    this->close();
}


void Form::on_send_clicked()
{
    // 输入按 UTF-8 编码发送，不自动添加换行；实际通信通道由主窗口决定。
    const QByteArray data =
        ui->textEditsend->toPlainText().toUtf8();

    emit sendmsg(data);
}
