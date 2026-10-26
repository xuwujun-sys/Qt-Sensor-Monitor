#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    // QApplication 管理界面事件；主窗口构造时初始化通信、图表和数据库线程。
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    // 进入事件循环，按钮操作、数据接收和定时器回调由 Qt 在此调度。
    return a.exec();
}
