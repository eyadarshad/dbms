#ifndef WORKERMANAGER_H
#define WORKERMANAGER_H

#include <QObject>
#include <QTableWidget>
#include <QDate>
#include "databasehandler.h"

class WorkerManager : public QObject
{
    Q_OBJECT
public:
    explicit WorkerManager(DatabaseHandler *dbHandler, QObject *parent = nullptr);

    // Load all workers to the table widget
    bool loadWorkers(QTableWidget *tableWidget);

    // Search workers by name or contact number
    void searchWorkers(QTableWidget *tableWidget, const QString &searchText);

    // Add a new worker
    bool addWorker(const QString &name, const QString &contact, const QString &email,
                   const QString &status, double salary, const QDate &dateOfJoining);

    // Remove a worker
    bool removeWorker(int workerId);

    bool executeUpdate(const QString &queryStr, const QVariantList &params);
    bool populateTable(QTableWidget *table, const QString &queryStr);
    bool populateTableWithQuery(QTableWidget *table, QSqlQuery &query);

signals:
    void workersUpdated();

private:
    DatabaseHandler *m_dbHandler;
};

#endif // WORKERMANAGER_H
