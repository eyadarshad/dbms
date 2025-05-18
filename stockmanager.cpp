#include "stockmanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDateTime>

StockManager::StockManager(DatabaseHandler *dbHandler, QObject *parent)
    : QObject(parent)
    , m_dbHandler(dbHandler)
{
}

bool StockManager::loadStockItems(QTableWidget *tableWidget)
{
    if (!m_dbHandler->isConnected()) {
        qDebug() << "Database not connected";
        return false;
    }

    // Clear the table
    tableWidget->setRowCount(0);

    QSqlQuery query;
    query.prepare("SELECT product_id, product_name, price, category, quantity "
                  "FROM Products ORDER BY product_name");

    if (!query.exec()) {
        qDebug() << "Failed to load stock items:" << query.lastError().text();
        return false;
    }

    qDebug() << "Loading stock items, rows returned: " << query.size();

    int row = 0;
    while (query.next()) {
        tableWidget->insertRow(row);

        int productId = query.value("product_id").toInt();
        qDebug() << "Processing product ID: " << productId;

        int totalQuantity = query.value("quantity").toInt();
        qDebug() << "Total quantity: " << totalQuantity;

        // Temporarily bypass calculation to directly use quantity from Products table
        int remainingQuantity = totalQuantity;

        // Set data for each column
        tableWidget->setItem(row, 0, new QTableWidgetItem(QString::number(productId)));
        tableWidget->setItem(row, 1, new QTableWidgetItem(query.value("product_name").toString()));
        tableWidget->setItem(row, 2, new QTableWidgetItem(QString::number(query.value("price").toDouble())));
        tableWidget->setItem(row, 3, new QTableWidgetItem(query.value("category").toString()));
        tableWidget->setItem(row, 4, new QTableWidgetItem(QString::number(remainingQuantity)));

        row++;
    }

    qDebug() << "Stock items loaded: " << row << " rows";
    return true;
}

void StockManager::searchStockItems(QTableWidget *tableWidget, const QString &searchText)
{
    if (!m_dbHandler->isConnected()) {
        qDebug() << "Database not connected";
        return;
    }

    // Clear the table
    tableWidget->setRowCount(0);

    QSqlQuery query;
    query.prepare("SELECT product_id, product_name, price, category, quantity "
                  "FROM Products WHERE product_name LIKE :search OR category LIKE :search "
                  "ORDER BY product_name");
    query.bindValue(":search", "%" + searchText + "%");

    if (!query.exec()) {
        qDebug() << "Failed to search stock items:" << query.lastError().text();
        return;
    }

    int row = 0;
    while (query.next()) {
        tableWidget->insertRow(row);

        int productId = query.value("product_id").toInt();
        int totalQuantity = query.value("quantity").toInt();
        int remainingQuantity = calculateRemainingQuantity(productId, totalQuantity);

        // Set data for each column
        tableWidget->setItem(row, 0, new QTableWidgetItem(QString::number(productId)));
        tableWidget->setItem(row, 1, new QTableWidgetItem(query.value("product_name").toString()));
        tableWidget->setItem(row, 2, new QTableWidgetItem(QString::number(query.value("price").toDouble())));
        tableWidget->setItem(row, 3, new QTableWidgetItem(query.value("category").toString()));
        tableWidget->setItem(row, 4, new QTableWidgetItem(QString::number(remainingQuantity)));

        row++;
    }
}


int StockManager::calculateRemainingQuantity(int productId, int totalQuantity)
{
    // This method calculates the remaining quantity by subtracting the sold quantity
    // from the total quantity in the Products table

    int soldQuantity = 0;

    // Check if SaleItems table exists and contains data
    QSqlQuery checkTableQuery;
    checkTableQuery.prepare("SHOW TABLES LIKE 'SaleItems'");

    bool tableExists = false;
    if (checkTableQuery.exec()) {
        tableExists = checkTableQuery.next();
    }

    if (tableExists) {
        // SaleItems table exists, query for sold quantity
        QSqlQuery query;
        query.prepare("SELECT SUM(quantity) as sold_quantity FROM SaleItems WHERE product_id = :productId");
        query.bindValue(":productId", productId);

        if (query.exec() && query.next()) {
            QVariant soldQtyVariant = query.value("sold_quantity");
            if (!soldQtyVariant.isNull()) {
                soldQuantity = soldQtyVariant.toInt();
            }
        } else {
            qDebug() << "Failed to query sold quantity:" << query.lastError().text();
        }
    }

    // Calculate remaining quantity
    int remainingQty = totalQuantity - soldQuantity;

    // Ensure we don't return negative quantity
    return (remainingQty < 0) ? 0 : remainingQty;
}
