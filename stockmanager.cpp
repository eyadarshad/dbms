#include "stockmanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QTableWidgetItem>

StockManager::StockManager(DatabaseHandler *dbHandler, QObject *parent)
    : QObject(parent)
    , m_dbHandler(dbHandler)
{
}

bool StockManager::loadStock(QTableWidget *tableWidget)
{
    if (!m_dbHandler->isConnected()) {
        qDebug() << "Database not connected";
        return false;
    }

    tableWidget->setRowCount(0);

    QSqlQuery query;
    query.prepare("SELECT p.product_id, p.product_name, p.price, p.category, "
                  "SUM(p.quantity) as total_quantity, "
                  "COALESCE(SUM(p.quantity) - COALESCE(s.sold_quantity, 0), SUM(p.quantity)) as remaining_quantity "
                  "FROM Products p "
                  "LEFT JOIN (SELECT product_id, SUM(quantity_sold) as sold_quantity FROM Sales GROUP BY product_id) s "
                  "ON p.product_id = s.product_id "
                  "GROUP BY p.product_id, p.product_name, p.price, p.category "
                  "ORDER BY p.product_name");

    if (!query.exec()) {
        qDebug() << "Failed to load stock:" << query.lastError().text();
        return false;
    }

    int row = 0;
    while (query.next()) {
        tableWidget->insertRow(row);

        tableWidget->setItem(row, 0, new QTableWidgetItem(query.value("product_id").toString()));
        tableWidget->setItem(row, 1, new QTableWidgetItem(query.value("product_name").toString()));
        tableWidget->setItem(row, 2, new QTableWidgetItem(query.value("price").toString()));
        tableWidget->setItem(row, 3, new QTableWidgetItem(query.value("category").toString()));
        tableWidget->setItem(row, 4, new QTableWidgetItem(query.value("total_quantity").toString()));
        tableWidget->setItem(row, 5, new QTableWidgetItem(query.value("remaining_quantity").toString()));

        row++;
    }

    return true;
}

void StockManager::searchStock(QTableWidget *tableWidget, const QString &searchText)
{
    if (!m_dbHandler->isConnected()) {
        qDebug() << "Database not connected";
        return;
    }

    tableWidget->setRowCount(0);

    QSqlQuery query;
    query.prepare("SELECT p.product_id, p.product_name, p.price, p.category, "
                  "SUM(p.quantity) as total_quantity, "
                  "COALESCE(SUM(p.quantity) - COALESCE(s.sold_quantity, 0), SUM(p.quantity)) as remaining_quantity "
                  "FROM Products p "
                  "LEFT JOIN (SELECT product_id, SUM(quantity_sold) as sold_quantity FROM Sales GROUP BY product_id) s "
                  "ON p.product_id = s.product_id "
                  "WHERE p.product_name LIKE :search OR p.category LIKE :search "
                  "GROUP BY p.product_id, p.product_name, p.price, p.category "
                  "ORDER BY p.product_name");
    query.bindValue(":search", "%" + searchText + "%");

    if (!query.exec()) {
        qDebug() << "Failed to search stock:" << query.lastError().text();
        return;
    }

    int row = 0;
    while (query.next()) {
        tableWidget->insertRow(row);

        tableWidget->setItem(row, 0, new QTableWidgetItem(query.value("product_id").toString()));
        tableWidget->setItem(row, 1, new QTableWidgetItem(query.value("product_name").toString()));
        tableWidget->setItem(row, 2, new QTableWidgetItem(query.value("price").toString()));
        tableWidget->setItem(row, 3, new QTableWidgetItem(query.value("category").toString()));
        tableWidget->setItem(row, 4, new QTableWidgetItem(query.value("total_quantity").toString()));
        tableWidget->setItem(row, 5, new QTableWidgetItem(query.value("remaining_quantity").toString()));

        row++;
    }
}
