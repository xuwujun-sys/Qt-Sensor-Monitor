#ifndef HISTORYWIDGET_H
#define HISTORYWIDGET_H

#include <QWidget>

namespace Ui { class HistoryWidget; }
class QStandardItemModel;

// 显示历史记录并发出按钮请求，数据库操作由主窗口协调。
class HistoryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HistoryWidget(QWidget *parent = nullptr); // 创建表格模型、表头和按钮连接。
    ~HistoryWidget(); // 释放 UI，模型由 Qt 随窗口自动释放。
    QStandardItemModel* get_model() const; // 返回模型指针，供调用方读取或修改表格数据。

signals:
    void refreshRequested(); // 点击“刷新”时发出，主窗口收到后查询数据库。
    void clearRequested();   // 点击“清空”时发出，当前尚未连接清空操作。

private:
    Ui::HistoryWidget *ui; // 访问历史表格及刷新、清空、关闭按钮。
    QStandardItemModel *m_model; // 窗口拥有模型，调用方通过 get_model() 填充数据。
};

#endif // HISTORYWIDGET_H
