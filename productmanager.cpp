#include "productmanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDateTime>

ProductManager::ProductManager(DatabaseHandler *dbHandler, QObject *parent)
    : QObject(parent)
    , m_dbHandler(dbHandler)
{
}

bool ProductManager::loadProducts(QTableWidget *tableWidget)
{
    if (!m_dbHandler->isConnected()) {
        qDebug() << "Database not connected";
        return false;
    }

    // Clear the table
    tableWidget->setRowCount(0);

    QSqlQuery query;
    query.prepare("SELECT product_id, product_name, price, category, quantity, updated_at "
                  "FROM Products ORDER BY product_name");

    if (!query.exec()) {
        qDebug() << "Failed to load products:" << query.lastError().text();
        return false;
    }

    int row = 0;
    while (query.next()) {
        tableWidget->insertRow(row);

        // Set data for each column
        tableWidget->setItem(row, 0, new QTableWidgetItem(query.value("product_id").toString()));
        tableWidget->setItem(row, 1, new QTableWidgetItem(query.value("product_name").toString()));
        tableWidget->setItem(row, 2, new QTableWidgetItem(query.value("price").toString()));
        tableWidget->setItem(row, 3, new QTableWidgetItem(query.value("category").toString()));
        tableWidget->setItem(row, 4, new QTableWidgetItem(query.value("quantity").toString()));

        // Format date
        QDateTime dateTime = query.value("updated_at").toDateTime();
        tableWidget->setItem(row, 5, new QTableWidgetItem(dateTime.toString("yyyy-MM-dd")));

        row++;
    }

    return true;
}

void ProductManager::searchProducts(QTableWidget *tableWidget, const QString &searchText)
{
    if (!m_dbHandler->isConnected()) {
        qDebug() << "Database not connected";
        return;
    }

    // Clear the table
    tableWidget->setRowCount(0);

    QSqlQuery query;
    query.prepare("SELECT product_id, product_name, price, category, quantity, updated_at "
                  "FROM Products WHERE product_name LIKE :search OR category LIKE :search "
                  "ORDER BY product_name");
    query.bindValue(":search", "%" + searchText + "%");

    if (!query.exec()) {
        qDebug() << "Failed to search products:" << query.lastError().text();
        return;
    }

    int row = 0;
    while (query.next()) {
        tableWidget->insertRow(row);

        // Set data for each column
        tableWidget->setItem(row, 0, new QTableWidgetItem(query.value("product_id").toString()));
        tableWidget->setItem(row, 1, new QTableWidgetItem(query.value("product_name").toString()));
        tableWidget->setItem(row, 2, new QTableWidgetItem(query.value("price").toString()));
        tableWidget->setItem(row, 3, new QTableWidgetItem(query.value("category").toString()));
        tableWidget->setItem(row, 4, new QTableWidgetItem(query.value("quantity").toString()));

        // Format date
        QDateTime dateTime = query.value("updated_at").toDateTime();
        tableWidget->setItem(row, 5, new QTableWidgetItem(dateTime.toString("yyyy-MM-dd")));

        row++;
    }
}

bool ProductManager::addProduct(const QString &name, double price, const QString &category,
                                int quantity, const QDate &dateAdded)
{
    if (!m_dbHandler->isConnected()) {
        qDebug() << "Database not connected";
        return false;
    }

    // Fix 1: Using direct INSERT instead of calling a stored procedure
    QSqlQuery query;
    query.prepare("INSERT INTO Products (product_name, price, category, quantity, updated_at) "
                  "VALUES (:name, :price, :category, :quantity, :date)");
    query.bindValue(":name", name);
    query.bindValue(":price", price);
    query.bindValue(":category", category);
    query.bindValue(":quantity", quantity);
    query.bindValue(":date", dateAdded);

    if (!query.exec()) {
        qDebug() << "Failed to add product:" << query.lastError().text();
        return false;
    }

    emit productsUpdated();
    return true;
}

bool ProductManager::removeProduct(int productId)
{
    if (!m_dbHandler->isConnected()) {
        qDebug() << "Database not connected";
        return false;
    }

    QSqlQuery query;
    query.prepare("DELETE FROM Products WHERE product_id = :id");
    query.bindValue(":id", productId);

    if (!query.exec()) {
        qDebug() << "Failed to remove product:" << query.lastError().text();
        return false;
    }

    emit productsUpdated();
    return true;
}

bool ProductManager::getProductStats(int &totalProducts, int &totalStock)
{
    if (!m_dbHandler->isConnected()) {
        qDebug() << "Database not connected";
        return false;
    }

    // Get total products and total stock
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) as product_count, SUM(quantity) as total_stock FROM Products");

    if (!query.exec()) {
        qDebug() << "Failed to get product stats:" << query.lastError().text();
        return false;
    }

    if (query.next()) {
        totalProducts = query.value("product_count").toInt();
        totalStock = query.value("total_stock").toInt();
        return true; // Fix 2: Return true on success
    }

    return false;
}
