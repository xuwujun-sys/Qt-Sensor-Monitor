#ifndef FORM_H
#define FORM_H

#include <QWidget>

namespace Ui {
class Form;
}

// 调试窗口：显示接收内容，提供手动输入和发送入口。
class Form : public QWidget
{
    Q_OBJECT

public:
    explicit Form(QWidget *parent = nullptr); // 创建调试界面。
    Ui::Form* get_ui(); // 返回 UI 指针，供主窗口向接收区追加消息。
    ~Form(); // 释放 UI 对象。

private slots:
    void on_exit_clicked(); // 关闭调试窗口，不退出主程序。

    void on_send_clicked(); // 将输入内容转为 UTF-8，通过 sendmsg 发出。

private:
    Ui::Form *ui; // 访问接收区、发送区和按钮。

signals:
    void sendmsg(QByteArray msg); // 请求发送原始字节，实际 TCP 发送由 Tcpmanger 处理。
};

#endif // FORM_H
