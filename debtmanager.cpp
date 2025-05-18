#include "debtmanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDateTime>

DebtManager::DebtManager(DatabaseHandler *dbHandler, QObject *parent)
    : QObject(parent)
    , m_dbHandler(dbHandler)
{
}

bool DebtManager::loadDebtors(QTableWidget *tableWidget)
{
    if (!m_dbHandler->isConnected()) {
        qDebug() << "Database not connected";
        return false;
    }

    // Clear the table
    tableWidget->setRowCount(0);

    QSqlQuery query;
    query.prepare("SELECT debtor_id, name, contact_number, email, address, debt_amount, date_incurred "
                  "FROM Debtors ORDER BY name");

    if (!query.exec()) {
        qDebug() << "Failed to load debtors:" << query.lastError().text();
        return false;
    }

    int row = 0;
    while (query.next()) {
        tableWidget->insertRow(row);

        // Set data for each column
        tableWidget->setItem(row, 0, new QTableWidgetItem(query.value("debtor_id").toString()));
        tableWidget->setItem(row, 1, new QTableWidgetItem(query.value("name").toString()));
        tableWidget->setItem(row, 2, new QTableWidgetItem(query.value("contact_number").toString()));
        tableWidget->setItem(row, 3, new QTableWidgetItem(query.value("email").toString()));
        tableWidget->setItem(row, 4, new QTableWidgetItem(query.value("address").toString()));
        tableWidget->setItem(row, 5, new QTableWidgetItem(query.value("debt_amount").toString()));

        // Format date
        QDate date = query.value("date_incurred").toDate();
        tableWidget->setItem(row, 6, new QTableWidgetItem(date.toString("yyyy-MM-dd")));

        row++;
    }

    return true;
}


void DebtManager::searchDebtors(QTableWidget *tableWidget, const QString &searchText)
{
    if (!m_dbHandler->isConnected()) {
        qDebug() << "Database not connected";
        return;
    }

    // Clear the table
    tableWidget->setRowCount(0);

    QSqlQuery query;
    query.prepare("SELECT debtor_id, name, contact_number, email, address, debt_amount, date_incurred "
                  "FROM Debtors WHERE name LIKE :search OR contact_number LIKE :search "
                  "OR email LIKE :search OR address LIKE :search "
                  "ORDER BY name");
    query.bindValue(":search", "%" + searchText + "%");

    if (!query.exec()) {
        qDebug() << "Failed to search debtors:" << query.lastError().text();
        return;
    }

    int row = 0;
    while (query.next()) {
        tableWidget->insertRow(row);

        // Set data for each column
        tableWidget->setItem(row, 0, new QTableWidgetItem(query.value("debtor_id").toString()));
        tableWidget->setItem(row, 1, new QTableWidgetItem(query.value("name").toString()));
        tableWidget->setItem(row, 2, new QTableWidgetItem(query.value("contact_number").toString()));
        tableWidget->setItem(row, 3, new QTableWidgetItem(query.value("email").toString()));
        tableWidget->setItem(row, 4, new QTableWidgetItem(query.value("address").toString()));
        tableWidget->setItem(row, 5, new QTableWidgetItem(query.value("debt_amount").toString()));

        // Format date
        QDate date = query.value("date_incurred").toDate();
        tableWidget->setItem(row, 6, new QTableWidgetItem(date.toString("yyyy-MM-dd")));

        row++;
    }
}

bool DebtManager::addDebtor(const QString &name, const QString &contact, const QString &email,
                            const QString &address, double debtAmount, const QDate &dateIncurred)
{
    if (!m_dbHandler->isConnected()) {
        qDebug() << "Database not connected";
        return false;
    }

    QSqlQuery query;
    query.prepare("CALL AddDebtor(:name, :contact, :email, :address, :amount, :date)");
    query.bindValue(":name", name);
    query.bindValue(":contact", contact);
    query.bindValue(":email", email);
    query.bindValue(":address", address);
    query.bindValue(":amount", debtAmount);
    query.bindValue(":date", dateIncurred);

    if (!query.exec()) {
        qDebug() << "Failed to add debtor:" << query.lastError().text();
        return false;
    }

    emit debtorsUpdated();
    return true;
}

bool DebtManager::removeDebtor(int debtorId)
{
    if (!m_dbHandler->isConnected()) {
        qDebug() << "Database not connected";
        return false;
    }

    QSqlQuery query;
    query.prepare("DELETE FROM Debtors WHERE debtor_id = :id");
    query.bindValue(":id", debtorId);

    if (!query.exec()) {
        qDebug() << "Failed to remove debtor:" << query.lastError().text();
        return false;
    }

    emit debtorsUpdated();
    return true;
}

bool DebtManager::getDebtorStats(int &totalDebtors, double &totalDebt)
{
    if (!m_dbHandler->isConnected()) {
        qDebug() << "Database not connected";
        return false;
    }

    QSqlQuery query;
    query.prepare("SELECT COUNT(*) as debtor_count, SUM(debt_amount) as total_debt FROM Debtors");

    if (!query.exec()) {
        qDebug() << "Failed to get debtor stats:" << query.lastError().text();
        return false;
    }

    if (query.next()) {
        totalDebtors = query.value("debtor_count").toInt();
        totalDebt = query.value("total_debt").toDouble();
        return true;
    }

    return false;
}
