#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_form.h"
#include "QTcpServer"
#include "QTcpSocket"
#include <QNetworkProxy>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_form(new Form)
    , m_server(new QTcpServer)
    , m_socket(new QTcpSocket)
    , m_socket_l(QVector<QTcpSocket*>())
    , m_tcpphraser(new TcpPhraser)
{
    Ui::Form* fui = m_form->get_ui();
    ui->setupUi(this);
    ui->netlineEditip->setText("127.0.0.1");
    ui->netlineEdithost->setText("9494");
    m_socket->setProxy(QNetworkProxy::NoProxy);

    {
        m_clientReadConnection = connect(m_socket, &QTcpSocket::readyRead, this, [=]() {

            const QByteArray data = m_socket->readAll();
            fui->textBrowserrecv->append("服务器收到:" + QString::fromUtf8(data));
            m_tcpphraser->append_data(data);
            m_tcpphraser->analyze();

        });

        m_sendConnection = connect(m_form, &Form::sendmsg, this, [=](QByteArray msg){
                m_socket->write(msg);
        });

        ui->actionnetbtn->setIcon(QIcon(":/img/client.png"));
        ui->actionnetbtn->setToolTip("当前：客户端；点击切换为服务器");
        isnetopen = false;
        m_socket->connectToHost(QHostAddress(ui->netlineEditip->text()), ui->netlineEdithost->text().toUShort());
    }

    //连接工具栏的开关和点击事件
    connect(ui->actionnetbtn, &QAction::triggered, this, [this, fui](){
        disconnect(m_serverConnection);
        disconnect(m_sendConnection);
        disconnect(m_clientReadConnection);

        if(isnetopen == false)
        {
            m_socket->close();

            // 创建服务器

            m_server->listen(QHostAddress(ui->netlineEditip->text()), ui->netlineEdithost->text().toUInt());

            // 服务器接收连接
            m_serverConnection = connect(m_server, &QTcpServer::newConnection, this, [=]() {

                m_socket_l.append(m_server->nextPendingConnection());
                const int index = m_socket_l_count;
                m_socket_l_count++;
                // 服务器接收数据
                connect(m_socket_l[index], &QTcpSocket::readyRead, this, [=]() {
                    const QByteArray data = m_socket_l[index]->readAll();
                    fui->textBrowserrecv->append("服务器收到:" + QString::fromUtf8(data));
                    m_tcpphraser->append_data(data);
                    m_tcpphraser->analyze();

                });

                disconnect(m_sendConnection);
                m_sendConnection = connect(m_form, &Form::sendmsg, this, [=](QByteArray msg){
                        m_socket_l[index]->write(msg);
                });

            });

            ui->actionnetbtn->setIcon(QIcon(":/img/server.png"));
            ui->actionnetbtn->setToolTip("当前：服务器；点击切换为客户端");
            isnetopen = true;
        }
        else
        {
            Ui::Form* fui = m_form->get_ui();
            //关闭服务器
            m_server->close();
            for(auto item : m_socket_l)
            {
                item->close();
            }

            m_socket->connectToHost(QHostAddress(ui->netlineEditip->text()), ui->netlineEdithost->text().toUInt());

            m_clientReadConnection = connect(m_socket, &QTcpSocket::readyRead, this, [=]() {

                const QByteArray data = m_socket->readAll();
                fui->textBrowserrecv->append("服务器收到:" + QString::fromUtf8(data));
                m_tcpphraser->append_data(data);
                m_tcpphraser->analyze();

            });

            m_sendConnection = connect(m_form, &Form::sendmsg, this, [=](QByteArray msg){
                    m_socket->write(msg);
            });

            ui->actionnetbtn->setIcon(QIcon(":/img/client.png"));
            ui->actionnetbtn->setToolTip("当前：客户端；点击切换为服务器");
            isnetopen = false;
        }

    });

    //连接工具栏的debug图标和点击事件
    connect(ui->actiondebug, &QAction::triggered, this, [&](){
            m_form->show();
    });

    //连接解析完成湿度等并且跟新标签
    connect(m_tcpphraser, &TcpPhraser::sensorDataParsed,
            this, [this](double temp,
                         double humi,
                         double light,
                         double soil,
                         double mq2,
                         double rain)
    {
        ui->temperatureValueLabel->setText(QString::number(temp));
        ui->humidityValueLabel->setText(QString::number(humi));
        ui->lightValueLabel->setText(QString::number(light));
        ui->soilValueLabel->setText(QString::number(soil));
        ui->smokeValueLabel->setText(QString::number(mq2));
        ui->rainValueLabel->setText(QString::number(rain));
    });


}

MainWindow::~MainWindow()
{
    delete ui;
}

