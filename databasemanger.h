#ifndef DATABASEMANGER_H
#define DATABASEMANGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QList>
#include <QVariantMap>
#include "sensordata.h"

// 管理 MySQL 连接与传感器记录的增删改查，不操作界面。
class DataBaseManger : public QObject
{
    Q_OBJECT
public:
    explicit DataBaseManger(QObject *parent = nullptr); // 创建对象，数据库连接延后到工作线程创建。
    ~DataBaseManger() override; // 关闭并移除本实例的数据库连接。

    // 以下普通方法只在本对象所属线程调用；主窗口通过信号请求槽函数。
    bool open(); // 尝试连接数据库，成功返回 true。

    bool insert(const SensorData &data); // 插入六项数值，采集时间和 id 由数据库生成。
    bool update(int id, const SensorData &data); // 按 id 更新六项数值，返回 SQL 是否执行成功。
    bool remove(int id); // 按 id 删除记录，返回 SQL 是否执行成功。
    bool select(QList<QVariantMap> &rows); // 查询全部记录，最新优先；结果写入 rows。

public slots:
    void initialize(); // 在线程启动后打开数据库，并查询一次历史记录。
    void saveData(SensorData data); // 在工作线程入库，成功后重新查询。
    void queryData(); // 在工作线程查询，将结果通过 dataReady 返回。

signals:
    void dataReady(QList<QVariantMap> rows); // 只传数据，表格仍由主线程更新。

private:
    QSqlDatabase m_db; // 当前实例使用的命名连接，供增删改查共用。

private:
    void init(); // 设置驱动、连接名、数据库地址及账号密码。
    bool execute(QSqlQuery &query, const QString &sql, const QVariantMap &values); // 绑定参数并执行 SQL。


};

#endif // DATABASEMANGER_H
