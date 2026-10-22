#ifndef DATABASEMANGER_H
#define DATABASEMANGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QList>
#include <QVariantMap>
#include "sensordata.h"

class DataBaseManger : public QObject
{
    Q_OBJECT
public:
    explicit DataBaseManger(QObject *parent = nullptr);
    ~DataBaseManger() override;

    bool open();

    bool insert(const SensorData &data);
    bool update(int id, const SensorData &data);
    bool remove(int id);
    bool select(QList<QVariantMap> &rows);

private:
    QSqlDatabase m_db;

private:
    void init();
    bool execute(QSqlQuery &query, const QString &sql, const QVariantMap &values);


};

#endif // DATABASEMANGER_H
