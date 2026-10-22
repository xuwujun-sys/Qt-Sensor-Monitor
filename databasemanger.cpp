#include "databasemanger.h"
#include <QSqlRecord>
#include <QUuid>

DataBaseManger::DataBaseManger(QObject *parent) : QObject(parent)
{
    init();
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
    m_db = QSqlDatabase::addDatabase("QODBC", QUuid::createUuid().toString());
    m_db.setDatabaseName("Driver={MySQL ODBC 8.0 Unicode Driver};"
                       "Server=127.0.0.1;Port=3306;Database=testindex;");
    m_db.setUserName("root");
    m_db.setPassword(qEnvironmentVariable("MYSQL_PASSWORD"));
}

bool DataBaseManger::open()
{
    return m_db.open();
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
    query.prepare(sql);

    for (auto it = values.constBegin(); it != values.constEnd(); ++it)
        query.bindValue(it.key(), it.value());

    return query.exec();
}

bool DataBaseManger::select(QList<QVariantMap> &rows)
{
    rows.clear();
    QSqlQuery query(m_db);
    const bool result = execute(query, "SELECT id, collect_time, temperature, humidity, light, soil, smoke, rain "
                                       "FROM sensordata ORDER BY collect_time DESC, id DESC", {});

    const QSqlRecord record = query.record();
    while (query.next())
    {
        QVariantMap row;
        for (int i = 0; i < record.count(); ++i)
            row.insert(record.fieldName(i), query.value(i));
        rows.append(row);
    }

    return result;
}
