#ifndef HISTORYWIDGET_H
#define HISTORYWIDGET_H

#include <QWidget>

namespace Ui { class HistoryWidget; }
class QStandardItemModel;

class HistoryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HistoryWidget(QWidget *parent = nullptr);
    ~HistoryWidget();
    QStandardItemModel* get_model() const;

signals:
    // 后续接数据库时，在这里连接刷新和清空操作。
    void refreshRequested();
    void clearRequested();

private:
    Ui::HistoryWidget *ui;
    QStandardItemModel *m_model; // 表格的数据模型，由窗口自动释放。
};

#endif // HISTORYWIDGET_H
