#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "form.h"
#include "QTcpServer"
#include "QTcpSocket"
#include "tcpphraser.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    bool isnetopen = false;
    Form* m_form;

    QTcpServer* m_server;
    QTcpSocket* m_socket;
    QVector<QTcpSocket*> m_socket_l;

    QMetaObject::Connection m_serverConnection;
    QMetaObject::Connection m_sendConnection;
    QMetaObject::Connection m_clientReadConnection;
    int m_socket_l_count = 0;

    TcpPhraser* m_tcpphraser;
};
#endif // MAINWINDOW_H
