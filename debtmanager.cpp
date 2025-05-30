#include "debtmanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

DebtManager::DebtManager(DatabaseHandler *dbHandler, QObject *parent)
    : QObject(parent), m_dbHandler(dbHandler) {}

bool DebtManager::loadDebtors(QTableWidget *tableWidget)
{
    return populateTable(tableWidget,
                         "SELECT debtor_id, name, contact_number, address, debt_amount, date_incurred "
                         "FROM Debtors ORDER BY name");
}

void DebtManager::searchDebtors(QTableWidget *tableWidget, const QString &searchText)
{
    QSqlQuery query;
    query.prepare("SELECT debtor_id, name, contact_number, address, debt_amount, date_incurred "
                  "FROM Debtors WHERE CONCAT(name, contact_number, address) LIKE ? ORDER BY name");
    query.addBindValue("%" + searchText + "%");

    populateTableWithQuery(tableWidget, query);
}

bool DebtManager::addDebtor(const QString &name, const QString &contact,
                            const QString &address, double debtAmount, const QDate &dateIncurred)
{
    if (!m_dbHandler->isConnected()) return false;

    QSqlQuery query;
    query.prepare("INSERT INTO Debtors (name, contact_number, address, debt_amount, date_incurred) "
                  "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(name);
    query.addBindValue(contact);
    query.addBindValue(address);
    query.addBindValue(debtAmount);
    query.addBindValue(dateIncurred);

    bool success = query.exec();
    if (success) emit debtorsUpdated();
    else qDebug() << "Failed to add debtor:" << query.lastError().text();
    return success;
}

bool DebtManager::removeDebtor(int debtorId)
{
    return executeQuery("DELETE FROM Debtors WHERE debtor_id = ?", {debtorId});
}

bool DebtManager::getDebtorStats(int &totalDebtors, double &totalDebt)
{
    if (!m_dbHandler->isConnected()) return false;

    QSqlQuery query("SELECT COUNT(*) as count, COALESCE(SUM(debt_amount), 0) as total FROM Debtors");
    if (query.exec() && query.next()) {
        totalDebtors = query.value(0).toInt();
        totalDebt = query.value(1).toDouble();
        return true;
    }
    return false;
}

// Private helper methods
bool DebtManager::populateTable(QTableWidget *tableWidget, const QString &sql)
{
    QSqlQuery query(sql);
    return populateTableWithQuery(tableWidget, query);
}

bool DebtManager::populateTableWithQuery(QTableWidget *tableWidget, QSqlQuery &query)
{
    if (!m_dbHandler->isConnected() || !query.exec()) {
        qDebug() << "Query failed:" << query.lastError().text();
        return false;
    }

    tableWidget->setRowCount(0);
    int row = 0;
    while (query.next()) {
        tableWidget->insertRow(row);
        for (int col = 0; col < 5; col++) {
            tableWidget->setItem(row, col, new QTableWidgetItem(query.value(col).toString()));
        }
        // Format date for last column
        tableWidget->setItem(row, 5, new QTableWidgetItem(
                                         query.value(5).toDate().toString("yyyy-MM-dd")));
        row++;
    }
    return true;
}

bool DebtManager::executeQuery(const QString &sql, const QVariantList &params)
{
    if (!m_dbHandler->isConnected()) return false;

    QSqlQuery query;
    query.prepare(sql);
    for (const auto &param : params) {
        query.addBindValue(param);
    }

    bool success = query.exec();
    if (success) emit debtorsUpdated();
    else qDebug() << "Query failed:" << query.lastError().text();
    return success;
}
