#include "databasemanger.h"
#include <QUuid>

DataBaseManger::DataBaseManger(QObject *parent) : QObject(parent)
{
}

DataBaseManger::~DataBaseManger()
{
    const QString connectionName = m_db.connectionName();
    m_db.close();
    m_db = QSqlDatabase();
    QSqlDatabase::removeDatabase(connectionName);
}

void DataBaseManger::init()
{
    // 每个实例使用独立连接名，避免多个对象共用默认连接。
    m_db = QSqlDatabase::addDatabase("QODBC", QUuid::createUuid().toString());
    m_db.setDatabaseName("Driver={MySQL ODBC 8.0 Unicode Driver};"
                       "Server=127.0.0.1;Port=3306;Database=testindex;");
    m_db.setUserName("root");
    m_db.setPassword("112233");
}

bool DataBaseManger::open()
{
    if (!m_db.isValid())
        init(); // 在实际调用 open() 的线程创建连接。
    return m_db.open();
}

void DataBaseManger::initialize()
{
    if (open())
        queryData();
}

void DataBaseManger::saveData(SensorData data)
{
    if (insert(data))
        queryData();
}

void DataBaseManger::queryData()
{
    QList<QVariantMap> rows;
    if (select(rows))
        emit dataReady(rows);
}

bool DataBaseManger::insert(const SensorData &data)
{
    QSqlQuery query(m_db);
    return execute(query, "INSERT INTO sensordata "
                  "(temperature, humidity, light, soil, smoke, rain) "
                  "VALUES (:temperature, :humidity, :light, :soil, :smoke, :rain)",
                  {{":temperature", data.temperature}, {":humidity", data.humidity},
                   {":light", data.light}, {":soil", data.soil},
                   {":smoke", data.smoke}, {":rain", data.rain}});
}

bool DataBaseManger::update(int id, const SensorData &data)
{
    QSqlQuery query(m_db);
    return execute(query, "UPDATE sensordata SET temperature=:temperature, humidity=:humidity, "
                  "light=:light, soil=:soil, smoke=:smoke, rain=:rain WHERE id=:id",
                  {{":id", id}, {":temperature", data.temperature},
                   {":humidity", data.humidity}, {":light", data.light},
                   {":soil", data.soil}, {":smoke", data.smoke}, {":rain", data.rain}});
}

bool DataBaseManger::remove(int id)
{
    QSqlQuery query(m_db);
    return execute(query, "DELETE FROM sensordata WHERE id=:id", {{":id", id}});
}

bool DataBaseManger::execute(QSqlQuery &query, const QString &sql,
                            const QVariantMap &values)
{
    // 数值通过参数绑定传入，不直接拼接进 SQL。
    query.prepare(sql);

    for (auto it = values.constBegin(); it != values.constEnd(); ++it)
        query.bindValue(it.key(), it.value());

    return query.exec();
}

bool DataBaseManger::select(QList<QVariantMap> &rows)
{
    rows.clear();
    QSqlQuery query(m_db);

    // 最新记录优先，同一采集时间按 id 排序。
    bool result = query.exec(
        "SELECT * FROM sensordata "
        "ORDER BY collect_time DESC, id DESC"
    );

    while (query.next()) {
        rows.append({
            {"id",           query.value("id")},
            {"collect_time", query.value("collect_time")},
            {"temperature",  query.value("temperature")},
            {"humidity",     query.value("humidity")},
            {"light",        query.value("light")},
            {"soil",         query.value("soil")},
            {"smoke",        query.value("smoke")},
            {"rain",         query.value("rain")}
        });
    }

    return result;
}
