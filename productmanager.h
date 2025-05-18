#ifndef PRODUCTMANAGER_H
#define PRODUCTMANAGER_H

#include <QObject>
#include <QTableWidget>
#include <QDate>
#include "databasehandler.h"

class ProductManager : public QObject
{
    Q_OBJECT
public:
    explicit ProductManager(DatabaseHandler *dbHandler, QObject *parent = nullptr);

    // Load all products to the table widget
    bool loadProducts(QTableWidget *tableWidget);

    // Search products by name or category
    void searchProducts(QTableWidget *tableWidget, const QString &searchText);

    // Add a new product
    bool addProduct(const QString &name, double price, const QString &category,
                    int quantity, const QDate &dateAdded);

    // Remove a product
    bool removeProduct(int productId);

    // Get dashboard stats
    bool getProductStats(int &totalProducts, int &totalStock);

signals:
    void productsUpdated();

private:
    DatabaseHandler *m_dbHandler;
};

#endif // PRODUCTMANAGER_H
