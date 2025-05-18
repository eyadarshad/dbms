#include "vendormanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDateTime>

VendorManager::VendorManager(DatabaseHandler *dbHandler, QObject *parent)
    : QObject(parent)
    , m_dbHandler(dbHandler)
{
}

bool VendorManager::loadVendors(QTableWidget *tableWidget)
{
    if (!m_dbHandler->isConnected()) {
        qDebug() << "Database not connected";
        return false;
    }

    // Clear the table
    tableWidget->setRowCount(0);

    QSqlQuery query;
    query.prepare("SELECT vendor_id, name, address, contact_number, cash_balance, date_of_supply "
                  "FROM Vendors ORDER BY name");

    if (!query.exec()) {
        qDebug() << "Failed to load vendors:" << query.lastError().text();
        return false;
    }

    int row = 0;
    while (query.next()) {
        tableWidget->insertRow(row);

        // Set data for each column
        tableWidget->setItem(row, 0, new QTableWidgetItem(query.value("vendor_id").toString()));
        tableWidget->setItem(row, 1, new QTableWidgetItem(query.value("name").toString()));
        tableWidget->setItem(row, 2, new QTableWidgetItem(query.value("address").toString()));
        tableWidget->setItem(row, 3, new QTableWidgetItem(query.value("cash_balance").toString()));
        tableWidget->setItem(row, 4, new QTableWidgetItem(query.value("contact_number").toString()));

        // Format date
        QDate date = query.value("date_of_supply").toDate();
        tableWidget->setItem(row, 5, new QTableWidgetItem(date.toString("yyyy-MM-dd")));

        row++;
    }

    return true;
}

void VendorManager::searchVendors(QTableWidget *tableWidget, const QString &searchText)
{
    if (!m_dbHandler->isConnected()) {
        qDebug() << "Database not connected";
        return;
    }

    // Clear the table
    tableWidget->setRowCount(0);

    QSqlQuery query;
    query.prepare("SELECT vendor_id, name, address, contact_number, cash_balance, date_of_supply "
                  "FROM Vendors WHERE name LIKE :search OR address LIKE :search "
                  "OR contact_number LIKE :search "
                  "ORDER BY name");
    query.bindValue(":search", "%" + searchText + "%");

    if (!query.exec()) {
        qDebug() << "Failed to search vendors:" << query.lastError().text();
        return;
    }

    int row = 0;
    while (query.next()) {
        tableWidget->insertRow(row);

        // Set data for each column
        tableWidget->setItem(row, 0, new QTableWidgetItem(query.value("vendor_id").toString()));
        tableWidget->setItem(row, 1, new QTableWidgetItem(query.value("name").toString()));
        tableWidget->setItem(row, 2, new QTableWidgetItem(query.value("address").toString()));
        tableWidget->setItem(row, 3, new QTableWidgetItem(query.value("cash_balance").toString()));
        tableWidget->setItem(row, 4, new QTableWidgetItem(query.value("contact_number").toString()));

        // Format date
        QDate date = query.value("date_of_supply").toDate();
        tableWidget->setItem(row, 5, new QTableWidgetItem(date.toString("yyyy-MM-dd")));

        row++;
    }
}

bool VendorManager::addVendor(const QString &name, const QString &address, const QString &contact,
                              double cashBalance, const QDate &dateOfSupply)
{
    if (!m_dbHandler->isConnected()) {
        qDebug() << "Database not connected";
        return false;
    }

    QSqlQuery query;
    query.prepare("CALL AddVendor(:name, :address, :contact, :cash, :date)");
    query.bindValue(":name", name);
    query.bindValue(":address", address);
    query.bindValue(":contact", contact);
    query.bindValue(":cash", cashBalance);
    query.bindValue(":date", dateOfSupply);

    if (!query.exec()) {
        qDebug() << "Failed to add vendor:" << query.lastError().text();
        return false;
    }

    emit vendorsUpdated();
    return true;
}

bool VendorManager::removeVendor(int vendorId)
{
    if (!m_dbHandler->isConnected()) {
        qDebug() << "Database not connected";
        return false;
    }

    QSqlQuery query;
    query.prepare("DELETE FROM Vendors WHERE vendor_id = :id");
    query.bindValue(":id", vendorId);

    if (!query.exec()) {
        qDebug() << "Failed to remove vendor:" << query.lastError().text();
        return false;
    }

    emit vendorsUpdated();
    return true;
}
