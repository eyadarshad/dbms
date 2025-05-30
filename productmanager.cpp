#include "productmanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

ProductManager::ProductManager(DatabaseHandler *dbHandler, QObject *parent)
    : QObject(parent), m_dbHandler(dbHandler) {}

bool ProductManager::loadProducts(QTableWidget *tableWidget)
{
    return populateTable(tableWidget,
                         "SELECT product_id, product_name, price, category, quantity, updated_at "
                         "FROM Products ORDER BY product_name");
}

void ProductManager::searchProducts(QTableWidget *tableWidget, const QString &searchText)
{
    QSqlQuery query;
    query.prepare("SELECT product_id, product_name, price, category, quantity, updated_at "
                  "FROM Products WHERE CONCAT(product_name, category) LIKE ? ORDER BY product_name");
    query.addBindValue("%" + searchText + "%");

    populateTableWithQuery(tableWidget, query);
}

bool ProductManager::addProduct(const QString &name, double price, const QString &category,
                                int quantity, const QDate &dateAdded)
{
    return executeQuery(
        "INSERT INTO Products (product_name, price, category, quantity, updated_at) VALUES (?, ?, ?, ?, ?)",
        {name, price, category, quantity, dateAdded});
}

bool ProductManager::removeProduct(int productId)
{
    return executeQuery("DELETE FROM Products WHERE product_id = ?", {productId});
}

bool ProductManager::getProductStats(int &totalProducts, int &totalStock)
{
    if (!m_dbHandler->isConnected()) return false;

    QSqlQuery query("SELECT COUNT(*) as count, COALESCE(SUM(quantity), 0) as stock FROM Products");
    if (query.exec() && query.next()) {
        totalProducts = query.value(0).toInt();
        totalStock = query.value(1).toInt();
        return true;
    }
    return false;
}

// Private helper methods
bool ProductManager::populateTable(QTableWidget *tableWidget, const QString &sql)
{
    QSqlQuery query(sql);
    return populateTableWithQuery(tableWidget, query);
}

bool ProductManager::populateTableWithQuery(QTableWidget *tableWidget, QSqlQuery &query)
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
        // Format datetime for last column
        tableWidget->setItem(row, 5, new QTableWidgetItem(
                                         query.value(5).toDateTime().toString("yyyy-MM-dd")));
        row++;
    }
    return true;
}

bool ProductManager::executeQuery(const QString &sql, const QVariantList &params)
{
    if (!m_dbHandler->isConnected()) return false;

    QSqlQuery query;
    query.prepare(sql);
    for (const auto &param : params) {
        query.addBindValue(param);
    }

    bool success = query.exec();
    if (success) emit productsUpdated();
    else qDebug() << "Query failed:" << query.lastError().text();
    return success;
}
