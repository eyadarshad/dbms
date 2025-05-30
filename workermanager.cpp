#include "workermanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

WorkerManager::WorkerManager(DatabaseHandler *dbHandler, QObject *parent)
    : QObject(parent), m_dbHandler(dbHandler) {}

bool WorkerManager::loadWorkers(QTableWidget *tableWidget)
{
    return populateTable(tableWidget,
                         "SELECT worker_id, name, contact_number, email, status, salary, date_of_joining "
                         "FROM Workers ORDER BY name");
}

void WorkerManager::searchWorkers(QTableWidget *tableWidget, const QString &searchText)
{
    QSqlQuery query;
    query.prepare("SELECT worker_id, name, contact_number, email, status, salary, date_of_joining "
                  "FROM Workers WHERE name LIKE :search OR contact_number LIKE :search "
                  "OR email LIKE :search ORDER BY name");
    query.bindValue(":search", "%" + searchText + "%");

    populateTableWithQuery(tableWidget, query);
}

bool WorkerManager::addWorker(const QString &name, const QString &contact, const QString &email,
                              const QString &status, double salary, const QDate &dateOfJoining)
{
    if (!m_dbHandler->isConnected()) return false;

    QSqlQuery query;
    query.prepare("INSERT INTO Workers (name, contact_number, email, status, salary, date_of_joining) "
                  "VALUES (:name, :contact, :email, :status, :salary, :date)");
    query.bindValue(":name", name);
    query.bindValue(":contact", contact);
    query.bindValue(":email", email);
    query.bindValue(":status", status);
    query.bindValue(":salary", salary);
    query.bindValue(":date", dateOfJoining);

    if (query.exec()) {
        emit workersUpdated();
        return true;
    }

    qDebug() << "Failed to add worker:" << query.lastError().text();
    return false;
}

bool WorkerManager::removeWorker(int workerId)
{
    return executeUpdate("DELETE FROM Workers WHERE worker_id = ?", {workerId});
}

// Private helper methods
bool WorkerManager::populateTable(QTableWidget *table, const QString &queryStr)
{
    QSqlQuery query;
    query.prepare(queryStr);
    return populateTableWithQuery(table, query);
}

bool WorkerManager::populateTableWithQuery(QTableWidget *table, QSqlQuery &query)
{
    if (!m_dbHandler->isConnected() || !query.exec()) {
        qDebug() << "Query failed:" << query.lastError().text();
        return false;
    }

    table->setRowCount(0);
    const QStringList columns = {"worker_id", "name", "contact_number", "email", "status", "salary", "date_of_joining"};
    const int dateColumn = 6;

    for (int row = 0; query.next(); ++row) {
        table->insertRow(row);
        for (int col = 0; col < columns.size(); ++col) {
            QString value = (col == dateColumn) ?
                                query.value(columns[col]).toDate().toString("yyyy-MM-dd") :
                                query.value(columns[col]).toString();
            table->setItem(row, col, new QTableWidgetItem(value));
        }
    }
    return true;
}

bool WorkerManager::executeUpdate(const QString &queryStr, const QVariantList &params)
{
    if (!m_dbHandler->isConnected()) return false;

    QSqlQuery query;
    query.prepare(queryStr);
    for (int i = 0; i < params.size(); ++i) {
        query.bindValue(i, params[i]);
    }

    if (query.exec()) {
        emit workersUpdated();
        return true;
    }

    qDebug() << "Update failed:" << query.lastError().text();
    return false;
}
