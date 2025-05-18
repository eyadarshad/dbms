#include "salesmanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDateTime>

SalesManager::SalesManager(DatabaseHandler *dbHandler, QObject *parent)
    : QObject(parent)
    , m_dbHandler(dbHandler)
{
}

bool SalesManager::loadSales(QTableWidget *tableWidget)
{
    if (!m_dbHandler->isConnected()) {
        qDebug() << "Database not connected";
        return false;
    }

    // Clear the table
    tableWidget->setRowCount(0);

    QSqlQuery query;
    query.prepare("SELECT sales_id, salesman_id, product_id, product_name, price, "
                  "category, quantity_sold, sale_date, total_price "
                  "FROM Sales ORDER BY sale_date DESC");

    if (!query.exec()) {
        qDebug() << "Failed to load sales:" << query.lastError().text();
        return false;
    }

    int row = 0;
    while (query.next()) {
        tableWidget->insertRow(row);

        // Set data for each column
        tableWidget->setItem(row, 0, new QTableWidgetItem(query.value("sales_id").toString()));
        tableWidget->setItem(row, 1, new QTableWidgetItem(query.value("salesman_id").toString()));
        tableWidget->setItem(row, 2, new QTableWidgetItem(query.value("product_id").toString()));
        tableWidget->setItem(row, 3, new QTableWidgetItem(query.value("product_name").toString()));
        tableWidget->setItem(row, 4, new QTableWidgetItem(query.value("price").toString()));
        tableWidget->setItem(row, 5, new QTableWidgetItem(query.value("category").toString()));
        tableWidget->setItem(row, 6, new QTableWidgetItem(query.value("quantity_sold").toString()));

        // Format date
        QDateTime dateTime = query.value("sale_date").toDateTime();
        tableWidget->setItem(row, 7, new QTableWidgetItem(dateTime.toString("yyyy-MM-dd hh:mm:ss")));

        row++;
    }

    return true;
}

void SalesManager::searchSales(QTableWidget *tableWidget, const QString &searchText)
{
    if (!m_dbHandler->isConnected()) {
        qDebug() << "Database not connected";
        return;
    }

    // Clear the table
    tableWidget->setRowCount(0);

    QSqlQuery query;
    query.prepare("SELECT sales_id, salesman_id, product_id, product_name, price, "
                  "category, quantity_sold, sale_date, total_price "
                  "FROM Sales WHERE product_name LIKE :search OR category LIKE :search "
                  "OR sales_id LIKE :search OR product_id LIKE :search "
                  "ORDER BY sale_date DESC");
    query.bindValue(":search", "%" + searchText + "%");

    if (!query.exec()) {
        qDebug() << "Failed to search sales:" << query.lastError().text();
        return;
    }

    int row = 0;
    while (query.next()) {
        tableWidget->insertRow(row);

        // Set data for each column
        tableWidget->setItem(row, 0, new QTableWidgetItem(query.value("sales_id").toString()));
        tableWidget->setItem(row, 1, new QTableWidgetItem(query.value("salesman_id").toString()));
        tableWidget->setItem(row, 2, new QTableWidgetItem(query.value("product_id").toString()));
        tableWidget->setItem(row, 3, new QTableWidgetItem(query.value("product_name").toString()));
        tableWidget->setItem(row, 4, new QTableWidgetItem(query.value("price").toString()));
        tableWidget->setItem(row, 5, new QTableWidgetItem(query.value("category").toString()));
        tableWidget->setItem(row, 6, new QTableWidgetItem(query.value("quantity_sold").toString()));

        // Format date
        QDateTime dateTime = query.value("sale_date").toDateTime();
        tableWidget->setItem(row, 7, new QTableWidgetItem(dateTime.toString("yyyy-MM-dd hh:mm:ss")));

        row++;
    }
}

bool SalesManager::processSale(const QList<SaleItem> &items, int userId, int debtorId)
{
    if (!m_dbHandler->isConnected()) {
        qDebug() << "Database not connected";
        return false;
    }

    // Start a transaction for atomic operations
    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();

    bool success = true;
    for (const SaleItem &item : items) {
        QSqlQuery query;

        // Get category for the product
        QString category;
        QSqlQuery categoryQuery;
        categoryQuery.prepare("SELECT category FROM Products WHERE product_id = :product_id");
        categoryQuery.bindValue(":product_id", item.productId);

        if (categoryQuery.exec() && categoryQuery.next()) {
            category = categoryQuery.value("category").toString();
        } else {
            category = "Unknown"; // Default if category not found
        }

        // Insert the sale record
        query.prepare("INSERT INTO Sales (salesman_id, product_id, product_name, price, category, "
                      "quantity_sold, total_price) "
                      "VALUES (:salesman_id, :product_id, :product_name, :price, :category, "
                      ":quantity_sold, :total_price)");
        query.bindValue(":salesman_id", userId);
        query.bindValue(":product_id", item.productId);
        query.bindValue(":product_name", item.productName);
        query.bindValue(":price", item.unitPrice);
        query.bindValue(":category", category);
        query.bindValue(":quantity_sold", item.quantity);
        query.bindValue(":total_price", item.totalPrice);

        if (!query.exec()) {
            qDebug() << "Failed to process sale:" << query.lastError().text();
            success = false;
            break;
        }

        // Update product inventory
        QSqlQuery updateQuery;
        updateQuery.prepare("UPDATE Products SET quantity = quantity - :sold_qty "
                            "WHERE product_id = :product_id");
        updateQuery.bindValue(":sold_qty", item.quantity);
        updateQuery.bindValue(":product_id", item.productId);

        if (!updateQuery.exec()) {
            qDebug() << "Failed to update product quantity:" << updateQuery.lastError().text();
            success = false;
            break;
        }
    }

    // Update debtor record if a debtor is specified
    if (success && debtorId > 0) {
        double totalAmount = 0;
        for (const SaleItem &item : items) {
            totalAmount += item.totalPrice;
        }

        QSqlQuery debtorQuery;
        debtorQuery.prepare("UPDATE Debtors SET debt_amount = debt_amount + :amount "
                            "WHERE debtor_id = :debtor_id");
        debtorQuery.bindValue(":amount", totalAmount);
        debtorQuery.bindValue(":debtor_id", debtorId);

        if (!debtorQuery.exec()) {
            qDebug() << "Failed to update debtor debt:" << debtorQuery.lastError().text();
            success = false;
        }
    }

    // Commit or rollback based on success
    if (success) {
        db.commit();
        emit salesUpdated();
        return true;
    } else {
        db.rollback();
        return false;
    }
}

bool SalesManager::getSalesStats(int &totalSales, double &totalAmount, double &profitMargin)
{
    if (!m_dbHandler->isConnected()) {
        qDebug() << "Database not connected";
        return false;
    }

    // Get sales stats
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) as total_sales, SUM(total_price) as total_amount FROM Sales");

    if (!query.exec()) {
        qDebug() << "Failed to get sales stats:" << query.lastError().text();
        return false;
    }

    if (query.next()) {
        totalSales = query.value("total_sales").toInt();
        totalAmount = query.value("total_amount").toDouble();
        profitMargin = 23.0; // Fixed profit margin of 23% based on your UI
        return true;
    }

    return false;
}
