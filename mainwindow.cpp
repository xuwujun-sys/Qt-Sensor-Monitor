#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "form.h"
#include "QTcpServer"
#include "QTcpSocket"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    //连接工具栏的开关和点击事件
    ui->setupUi(this);
    connect(ui->actionnetbtn, &QAction::triggered, this, [&](){
        if(isnetopen == false)
        {
            ui->actionnetbtn->setIcon(QIcon(":/img/open.png"));
            isnetopen = true;
        }
        else
        {
            ui->actionnetbtn->setIcon(QIcon(":/img/close.png"));
            isnetopen = false;
        }

    });

    //连接工具栏的debug图标和点击事件
    connect(ui->actiondebug, &QAction::triggered, this, [&](){
            Form* form = new Form();
            form->show();
    });

    // 创建服务器
    QTcpServer *server = new QTcpServer(this);
    server->listen(QHostAddress::LocalHost, 8080);

    // 服务器接收连接
    connect(server, &QTcpServer::newConnection, this, [=]() {

        QTcpSocket *socket = server->nextPendingConnection();

        // 服务器接收数据
        connect(socket, &QTcpSocket::readyRead, this, [=]() {
            qDebug() << "服务器收到:" << socket->readAll();

            socket->write("World");
        });
    });

}

MainWindow::~MainWindow()
{
    delete ui;
}

