#include "form.h"
#include "ui_form.h"

Form::Form(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form)
{
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
    const QByteArray data =
        ui->textEditsend->toPlainText().toUtf8();

    emit sendmsg(data);
}
