#ifndef HISTORYWIDGET_H
#define HISTORYWIDGET_H

#include <QWidget>

namespace Ui { class HistoryWidget; }

class HistoryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HistoryWidget(QWidget *parent = nullptr);
    ~HistoryWidget();

signals:
    // 后续接数据库时，在这里连接刷新和清空操作。
    void refreshRequested();
    void clearRequested();

private:
    Ui::HistoryWidget *ui;
};

#endif // HISTORYWIDGET_H
