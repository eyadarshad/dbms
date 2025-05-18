#include "workermanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDateTime>

WorkerManager::WorkerManager(DatabaseHandler *dbHandler, QObject *parent)
    : QObject(parent)
    , m_dbHandler(dbHandler)
{
}

bool WorkerManager::loadWorkers(QTableWidget *tableWidget)
{
    if (!m_dbHandler->isConnected()) {
        qDebug() << "Database not connected";
        return false;
    }

    // Clear the table
    tableWidget->setRowCount(0);

    QSqlQuery query;
    query.prepare("SELECT worker_id, name, contact_number, email, status, salary, date_of_joining "
                  "FROM Workers ORDER BY name");

    if (!query.exec()) {
        qDebug() << "Failed to load workers:" << query.lastError().text();
        return false;
    }

    int row = 0;
    while (query.next()) {
        tableWidget->insertRow(row);

        // Set data for each column
        tableWidget->setItem(row, 0, new QTableWidgetItem(query.value("worker_id").toString()));
        tableWidget->setItem(row, 1, new QTableWidgetItem(query.value("name").toString()));
        tableWidget->setItem(row, 2, new QTableWidgetItem(query.value("contact_number").toString()));
        tableWidget->setItem(row, 3, new QTableWidgetItem(query.value("email").toString()));
        tableWidget->setItem(row, 4, new QTableWidgetItem(query.value("status").toString()));
        tableWidget->setItem(row, 5, new QTableWidgetItem(query.value("salary").toString()));

        // Format date
        QDate date = query.value("date_of_joining").toDate();
        tableWidget->setItem(row, 6, new QTableWidgetItem(date.toString("yyyy-MM-dd")));

        row++;
    }

    return true;
}

void WorkerManager::searchWorkers(QTableWidget *tableWidget, const QString &searchText)
{
    if (!m_dbHandler->isConnected()) {
        qDebug() << "Database not connected";
        return;
    }

    // Clear the table
    tableWidget->setRowCount(0);

    QSqlQuery query;
    query.prepare("SELECT worker_id, name, contact_number, email, status, salary, date_of_joining "
                  "FROM Workers WHERE name LIKE :search OR contact_number LIKE :search "
                  "OR email LIKE :search "
                  "ORDER BY name");
    query.bindValue(":search", "%" + searchText + "%");

    if (!query.exec()) {
        qDebug() << "Failed to search workers:" << query.lastError().text();
        return;
    }

    int row = 0;
    while (query.next()) {
        tableWidget->insertRow(row);

        // Set data for each column
        tableWidget->setItem(row, 0, new QTableWidgetItem(query.value("worker_id").toString()));
        tableWidget->setItem(row, 1, new QTableWidgetItem(query.value("name").toString()));
        tableWidget->setItem(row, 2, new QTableWidgetItem(query.value("contact_number").toString()));
        tableWidget->setItem(row, 3, new QTableWidgetItem(query.value("email").toString()));
        tableWidget->setItem(row, 4, new QTableWidgetItem(query.value("status").toString()));
        tableWidget->setItem(row, 5, new QTableWidgetItem(query.value("salary").toString()));

        // Format date
        QDate date = query.value("date_of_joining").toDate();
        tableWidget->setItem(row, 6, new QTableWidgetItem(date.toString("yyyy-MM-dd")));

        row++;
    }
}

bool WorkerManager::addWorker(const QString &name, const QString &contact, const QString &email,
                              const QString &status, double salary, const QDate &dateOfJoining)
{
    if (!m_dbHandler->isConnected()) {
        qDebug() << "Database not connected";
        return false;
    }

    QSqlQuery query;
    query.prepare("CALL AddWorker(:name, :contact, :email, :status, :salary, :date)");
    query.bindValue(":name", name);
    query.bindValue(":contact", contact);
    query.bindValue(":email", email);
    query.bindValue(":status", status);
    query.bindValue(":salary", salary);
    query.bindValue(":date", dateOfJoining);

    if (!query.exec()) {
        qDebug() << "Failed to add worker:" << query.lastError().text();
        return false;
    }

    emit workersUpdated();
    return true;
}

bool WorkerManager::removeWorker(int workerId)
{
    if (!m_dbHandler->isConnected()) {
        qDebug() << "Database not connected";
        return false;
    }

    QSqlQuery query;
    query.prepare("DELETE FROM Workers WHERE worker_id = :id");
    query.bindValue(":id", workerId);

    if (!query.exec()) {
        qDebug() << "Failed to remove worker:" << query.lastError().text();
        return false;
    }

    emit workersUpdated();
    return true;
}
