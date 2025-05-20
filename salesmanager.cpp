#include "salesmanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDateTime>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include "DatabaseHandler.h"

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

        // Add total price column
        tableWidget->setItem(row, 8, new QTableWidgetItem(query.value("total_price").toString()));

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

        // Add total price column
        tableWidget->setItem(row, 8, new QTableWidgetItem(query.value("total_price").toString()));

        row++;
    }
}

bool SalesManager::searchProducts(QTableWidget *tableWidget, const QString &searchText)
{
    if (!m_dbHandler->isConnected()) {
        qDebug() << "Database not connected";
        return false;
    }

    // Clear the table
    tableWidget->setRowCount(0);

    QSqlQuery query;
    query.prepare("SELECT product_id, product_name, price, category, quantity "
                  "FROM Products WHERE product_name LIKE :search OR category LIKE :search "
                  "ORDER BY product_name");
    query.bindValue(":search", "%" + searchText + "%");

    if (!query.exec()) {
        qDebug() << "Failed to search products:" << query.lastError().text();
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

        row++;
    }

    return true;
}

bool SalesManager::getProductInfo(int productId, SaleItem &item)
{
    if (!m_dbHandler->isConnected()) {
        qDebug() << "Database not connected";
        return false;
    }

    QSqlQuery query;
    query.prepare("SELECT product_id, product_name, price, category, quantity "
                  "FROM Products WHERE product_id = :product_id");
    query.bindValue(":product_id", productId);

    if (!query.exec()) {
        qDebug() << "Failed to get product info:" << query.lastError().text();
        return false;
    }

    if (query.next()) {
        item.productId = query.value("product_id").toInt();
        item.productName = query.value("product_name").toString();
        item.unitPrice = query.value("price").toDouble();
        item.category = query.value("category").toString();
        item.available = query.value("quantity").toInt();
        item.quantity = 1; // Default quantity is 1
        item.totalPrice = item.unitPrice * item.quantity;
        return true;
    }

    return false;
}

bool SalesManager::getProductsForRecommendation(QWidget *parentWidget, const QString &searchText)
{
    if (!m_dbHandler->isConnected() || !parentWidget) {
        return false;
    }

    // Clear existing layout content
    QLayout *layout = parentWidget->layout();
    if (layout) {
        QLayoutItem *item;
        while ((item = layout->takeAt(0)) != nullptr) {
            if (item->widget()) {
                item->widget()->deleteLater();
            }
            delete item;
        }
    } else {
        layout = new QVBoxLayout(parentWidget);
        parentWidget->setLayout(layout);
    }

    QSqlQuery query;
    query.prepare("SELECT product_id, product_name, price, category, quantity "
                  "FROM Products WHERE product_name LIKE :search OR category LIKE :search "
                  "ORDER BY product_name LIMIT 5");
    query.bindValue(":search", "%" + searchText + "%");

    if (!query.exec()) {
        qDebug() << "Failed to get recommendations:" << query.lastError().text();
        return false;
    }

    // Add product recommendations to layout
    while (query.next()) {
        int productId = query.value("product_id").toInt();
        QString productName = query.value("product_name").toString();
        QString price = query.value("price").toString();

        QWidget *productWidget = new QWidget(parentWidget);
        QHBoxLayout *productLayout = new QHBoxLayout(productWidget);

        QLabel *nameLabel = new QLabel(productName, productWidget);
        QLabel *priceLabel = new QLabel(price + " Rs.", productWidget);
        priceLabel->setAlignment(Qt::AlignRight);

        productLayout->addWidget(nameLabel);
        productLayout->addWidget(priceLabel);

        productWidget->setProperty("productId", productId);
        productWidget->setProperty("productName", productName);
        productWidget->setProperty("price", price);
        productWidget->setProperty("category", query.value("category").toString());
        productWidget->setProperty("available", query.value("quantity").toInt());

        // Style the product widget
        productWidget->setStyleSheet("background-color: #2D2D2D; color: white; border-radius: 5px; padding: 8px;");
        productWidget->setCursor(Qt::PointingHandCursor);

        // Make clickable
        productWidget->installEventFilter(parentWidget);

        layout->addWidget(productWidget);
    }

    return true;
}

bool SalesManager::processSale(const QList<SaleItem> &items, int userId)
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

        // Insert the sale record
        query.prepare("INSERT INTO Sales (salesman_id, product_id, product_name, price, category, "
                      "quantity_sold, total_price) "
                      "VALUES (:salesman_id, :product_id, :product_name, :price, :category, "
                      ":quantity_sold, :total_price)");
        query.bindValue(":salesman_id", userId);
        query.bindValue(":product_id", item.productId);
        query.bindValue(":product_name", item.productName);
        query.bindValue(":price", item.unitPrice);
        query.bindValue(":category", item.category);
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
