#ifndef PRODUCTMANAGER_H
#define PRODUCTMANAGER_H

#include <QObject>
#include <QTableWidget>
#include <QDate>
#include <QSqlQuery>
#include "databasehandler.h"

class ProductManager : public QObject
{
    Q_OBJECT
public:
    explicit ProductManager(DatabaseHandler *dbHandler, QObject *parent = nullptr);

    bool loadProducts(QTableWidget *tableWidget);
    void searchProducts(QTableWidget *tableWidget, const QString &searchText);
    bool addProduct(const QString &name, double price, const QString &category,
                    int quantity, const QDate &dateAdded);
    bool removeProduct(int productId);
    bool getProductStats(int &totalProducts, int &totalStock);

signals:
    void productsUpdated();

private:
    DatabaseHandler *m_dbHandler;

    // Helper methods to reduce code duplication
    bool populateTable(QTableWidget *tableWidget, const QString &sql);
    bool populateTableWithQuery(QTableWidget *tableWidget, QSqlQuery &query);
    bool executeQuery(const QString &sql, const QVariantList &params = {});
};

#endif // PRODUCTMANAGER_H
