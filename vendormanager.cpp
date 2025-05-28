#include "vendormanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

VendorManager::VendorManager(DatabaseHandler *dbHandler, QObject *parent)
    : QObject(parent), m_dbHandler(dbHandler) {}

bool VendorManager::loadVendors(QTableWidget *tableWidget)
{
    return populateTable(tableWidget,
                         "SELECT vendor_id, name, contact_number, address, cash_balance, date_of_supply "
                         "FROM Vendors ORDER BY name");
}

void VendorManager::searchVendors(QTableWidget *tableWidget, const QString &searchText)
{
    QSqlQuery query;
    query.prepare("SELECT vendor_id, name, contact_number, address, cash_balance, date_of_supply "
                  "FROM Vendors WHERE name LIKE :search OR address LIKE :search "
                  "OR contact_number LIKE :search ORDER BY name");
    query.bindValue(":search", "%" + searchText + "%");

    populateTableWithQuery(tableWidget, query);
}

bool VendorManager::addVendor(const QString &name, const QString &address, const QString &contact,
                              double cashBalance, const QDate &dateOfSupply)
{
    if (!m_dbHandler->isConnected()) return false;

    QSqlQuery query;
    query.prepare("INSERT INTO Vendors (name, address, contact_number, cash_balance, date_of_supply) "
                  "VALUES (:name, :address, :contact, :cash, :date)");
    query.bindValue(":name", name);
    query.bindValue(":address", address);
    query.bindValue(":contact", contact);
    query.bindValue(":cash", cashBalance);
    query.bindValue(":date", dateOfSupply);

    if (query.exec()) {
        emit vendorsUpdated();
        return true;
    }

    qDebug() << "Failed to add vendor:" << query.lastError().text();
    return false;
}

bool VendorManager::removeVendor(int vendorId)
{
    return executeUpdate("DELETE FROM Vendors WHERE vendor_id = ?", {vendorId});
}

// Private helper methods
bool VendorManager::populateTable(QTableWidget *table, const QString &queryStr)
{
    QSqlQuery query;
    query.prepare(queryStr);
    return populateTableWithQuery(table, query);
}

bool VendorManager::populateTableWithQuery(QTableWidget *table, QSqlQuery &query)
{
    if (!m_dbHandler->isConnected() || !query.exec()) {
        qDebug() << "Query failed:" << query.lastError().text();
        return false;
    }

    table->setRowCount(0);
    const QStringList columns = {"vendor_id", "name", "contact_number", "address", "cash_balance", "date_of_supply"};
    const int dateColumn = 5;

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

bool VendorManager::executeUpdate(const QString &queryStr, const QVariantList &params)
{
    if (!m_dbHandler->isConnected()) return false;

    QSqlQuery query;
    query.prepare(queryStr);
    for (int i = 0; i < params.size(); ++i) {
        query.bindValue(i, params[i]);
    }

    if (query.exec()) {
        emit vendorsUpdated();
        return true;
    }

    qDebug() << "Update failed:" << query.lastError().text();
    return false;
}
