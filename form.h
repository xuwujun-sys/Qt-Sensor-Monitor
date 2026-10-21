#ifndef FORM_H
#define FORM_H

#include <QWidget>

namespace Ui {
class Form;
}

class Form : public QWidget
{
    Q_OBJECT

public:
    explicit Form(QWidget *parent = nullptr);
    Ui::Form* get_ui();
    ~Form();

private slots:
    void on_exit_clicked();

    void on_send_clicked();

private:
    Ui::Form *ui;

signals:
    void sendmsg(QByteArray msg);
};

#endif // FORM_H
