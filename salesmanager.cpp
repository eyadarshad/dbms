#include "salesmanager.h"
#include "clickableWidget.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDateTime>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include "DatabaseHandler.h"

SalesManager::SalesManager(DatabaseHandler *dbHandler, QObject *parent)
    : QObject(parent), m_dbHandler(dbHandler)
{
}

bool SalesManager::loadSales(QTableWidget *tableWidget)
{
    return executeSalesQuery(tableWidget,
                             "SELECT sales_id, salesman_id, product_id, product_name, price, "
                             "category, quantity_sold, sale_date, total_price "
                             "FROM Sales ORDER BY sale_date DESC");
}

void SalesManager::searchSales(QTableWidget *tableWidget, const QString &searchText)
{
    QSqlQuery query;
    query.prepare("SELECT sales_id, salesman_id, product_id, product_name, price, "
                  "category, quantity_sold, sale_date, total_price "
                  "FROM Sales WHERE product_name LIKE :search OR category LIKE :search "
                  "OR sales_id LIKE :search OR product_id LIKE :search "
                  "ORDER BY sale_date DESC");
    query.bindValue(":search", "%" + searchText + "%");

    executeSalesQuery(tableWidget, query);
}

bool SalesManager::searchProducts(QTableWidget *tableWidget, const QString &searchText)
{
    if (!m_dbHandler->isConnected()) return false;

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
        for (int col = 0; col < 5; col++) {
            tableWidget->setItem(row, col, new QTableWidgetItem(query.value(col).toString()));
        }
        row++;
    }

    return true;
}

bool SalesManager::getProductInfo(int productId, SaleItem &item)
{
    if (!m_dbHandler->isConnected()) return false;

    QSqlQuery query;
    query.prepare("SELECT product_id, product_name, price, category, quantity "
                  "FROM Products WHERE product_id = :product_id");
    query.bindValue(":product_id", productId);

    if (!query.exec() || !query.next()) {
        qDebug() << "Failed to get product info:" << query.lastError().text();
        return false;
    }

    item.productId = query.value(0).toInt();
    item.productName = query.value(1).toString();
    item.unitPrice = query.value(2).toDouble();
    item.category = query.value(3).toString();
    item.available = query.value(4).toInt();
    item.quantity = 1;
    item.totalPrice = item.unitPrice;

    return true;
}

bool SalesManager::getProductsForRecommendation(QVBoxLayout *layout, const QString &searchText)
{
    if (!m_dbHandler->isConnected() || !layout) return false;

    QSqlQuery query;
    query.prepare("SELECT product_id, product_name, price, category, quantity "
                  "FROM Products WHERE product_name LIKE :search AND quantity > 0 "
                  "ORDER BY product_name LIMIT 10");
    query.bindValue(":search", "%" + searchText + "%");

    if (!query.exec()) {
        qDebug() << "Failed to get recommendations:" << query.lastError().text();
        return false;
    }

    while (query.next()) {
        createProductWidget(layout, query);
    }

    return true;
}

void SalesManager::createProductWidget(QVBoxLayout *layout, const QSqlQuery &query)
{
    int productId = query.value(0).toInt();
    QString name = query.value(1).toString();
    double price = query.value(2).toDouble();
    QString category = query.value(3).toString();
    int quantity = query.value(4).toInt();

    // Don't create widgets for out-of-stock products
    if (quantity <= 0) return;

    auto *productWidget = new ClickableWidget(layout->parentWidget());
    productWidget->setObjectName(QString("product_%1").arg(productId));
    productWidget->setStyleSheet("background-color: #1e1e1e; color: white; border-radius: 5px; margin: 2px; padding: 5px;");
    productWidget->setCursor(Qt::PointingHandCursor);

    auto *productLayout = new QHBoxLayout(productWidget);
    productLayout->setContentsMargins(10, 5, 10, 5);

    auto *infoLayout = new QVBoxLayout();
    auto *nameLabel = new QLabel(name, productWidget);
    nameLabel->setStyleSheet("font-size: 14px; font-weight: bold;");
    auto *unitLabel = new QLabel(QString("Stock: %1 | %2 Rs./Unit").arg(quantity).arg(price, 0, 'f', 2), productWidget);
    unitLabel->setStyleSheet("font-size: 12px; color: #aaa;");

    infoLayout->addWidget(nameLabel);
    infoLayout->addWidget(unitLabel);

    auto *priceLabel = new QLabel(QString("%1 Rs.").arg(price, 0, 'f', 2), productWidget);
    priceLabel->setStyleSheet("font-size: 14px; font-weight: bold;");
    priceLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    productLayout->addLayout(infoLayout);
    productLayout->addStretch();
    productLayout->addWidget(priceLabel);

    // Store product data as properties
    productWidget->setProperty("productId", productId);
    productWidget->setProperty("productName", name);
    productWidget->setProperty("unitPrice", price);
    productWidget->setProperty("category", category);
    productWidget->setProperty("available", quantity);

    // Connect the click signal to emit productSelected
    connect(productWidget, &ClickableWidget::clicked, [this, productId, name, price, category, quantity]() {
        // Validate stock before emitting
        if (quantity > 0) {
            emit productSelectedFromWidget(productId, name, price, category, quantity);
        } else {
            // Show warning for out of stock
            QMessageBox::warning(nullptr, "Out of Stock",
                                 QString("Product '%1' is currently out of stock.").arg(name));
        }
    });

    layout->addWidget(productWidget);
}

bool SalesManager::processSale(const QList<SaleItem> &items, int userId)
{
    if (!m_dbHandler->isConnected()) return false;

    // Validate all items have stock before processing
    for (const auto &item : items) {
        QSqlQuery stockCheck;
        stockCheck.prepare("SELECT quantity FROM Products WHERE product_id = ?");
        stockCheck.addBindValue(item.productId);

        if (!stockCheck.exec() || !stockCheck.next()) {
            qDebug() << "Failed to check stock for product:" << item.productId;
            return false;
        }

        int availableStock = stockCheck.value(0).toInt();
        if (availableStock < item.quantity) {
            QMessageBox::warning(nullptr, "Insufficient Stock",
                                 QString("Product '%1' only has %2 units in stock, but you're trying to sell %3 units.")
                                     .arg(item.productName).arg(availableStock).arg(item.quantity));
            return false;
        }
    }

    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();

    QSqlQuery saleQuery, updateQuery;
    saleQuery.prepare("INSERT INTO Sales (salesman_id, product_id, product_name, price, category, "
                      "quantity_sold, total_price) VALUES (?, ?, ?, ?, ?, ?, ?)");
    updateQuery.prepare("UPDATE Products SET quantity = quantity - ? WHERE product_id = ?");

    for (const auto &item : items) {
        saleQuery.addBindValue(userId);
        saleQuery.addBindValue(item.productId);
        saleQuery.addBindValue(item.productName);
        saleQuery.addBindValue(item.unitPrice);
        saleQuery.addBindValue(item.category);
        saleQuery.addBindValue(item.quantity);
        saleQuery.addBindValue(item.totalPrice);

        if (!saleQuery.exec()) {
            qDebug() << "Failed to process sale:" << saleQuery.lastError().text();
            db.rollback();
            return false;
        }

        updateQuery.addBindValue(item.quantity);
        updateQuery.addBindValue(item.productId);

        if (!updateQuery.exec()) {
            qDebug() << "Failed to update inventory:" << updateQuery.lastError().text();
            db.rollback();
            return false;
        }
    }

    db.commit();
    emit salesUpdated();
    return true;
}

bool SalesManager::getSalesStats(int &totalSales, double &totalAmount, double &profitMargin)
{
    if (!m_dbHandler->isConnected()) return false;

    QSqlQuery query;
    query.prepare("SELECT COUNT(*) as total_sales, COALESCE(SUM(total_price), 0) as total_amount FROM Sales");

    if (!query.exec() || !query.next()) {
        qDebug() << "Failed to get sales stats:" << query.lastError().text();
        return false;
    }

    totalSales = query.value(0).toInt();
    totalAmount = query.value(1).toDouble();
    profitMargin = 23.0;

    return true;
}

bool SalesManager::executeSalesQuery(QTableWidget *tableWidget, const QString &queryStr)
{
    QSqlQuery query;
    query.prepare(queryStr);
    return executeSalesQuery(tableWidget, query);
}

bool SalesManager::executeSalesQuery(QTableWidget *tableWidget, QSqlQuery &query)
{
    if (!m_dbHandler->isConnected()) {
        qDebug() << "Database not connected";
        return false;
    }

    tableWidget->setRowCount(0);

    if (!query.exec()) {
        qDebug() << "Failed to execute sales query:" << query.lastError().text();
        return false;
    }

    int row = 0;
    while (query.next()) {
        tableWidget->insertRow(row);

        for (int col = 0; col < 8; col++) {
            QString value = (col == 7) ?
                                query.value("sale_date").toDateTime().toString("yyyy-MM-dd hh:mm:ss") :
                                query.value(col).toString();
            tableWidget->setItem(row, col, new QTableWidgetItem(value));
        }
        row++;
    }

    return true;
}
