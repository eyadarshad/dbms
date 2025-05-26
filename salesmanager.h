#ifndef SALESMANAGER_H
#define SALESMANAGER_H

#include <QObject>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QSqlQuery>
#include <QList>
#include "saleitem.h"
// Forward declarations
class DatabaseHandler;



class SalesManager : public QObject
{
    Q_OBJECT

public:
    explicit SalesManager(DatabaseHandler *dbHandler, QObject *parent = nullptr);

    // Sales operations
    bool loadSales(QTableWidget *tableWidget);
    void searchSales(QTableWidget *tableWidget, const QString &searchText);
    bool processSale(const QList<SaleItem> &items, int userId);
    bool getSalesStats(int &totalSales, double &totalAmount, double &profitMargin);

    // Product operations
    bool searchProducts(QTableWidget *tableWidget, const QString &searchText);
    bool getProductInfo(int productId, SaleItem &item);
    bool getProductsForRecommendation(QVBoxLayout *layout, const QString &searchText);

signals:
    void salesUpdated();
    void productSelectedFromWidget(int productId, QString productName, double price, QString category, int available);

private:
    void createProductWidget(QVBoxLayout *layout, const QSqlQuery &query);
    bool executeSalesQuery(QTableWidget *tableWidget, const QString &queryStr);
    bool executeSalesQuery(QTableWidget *tableWidget, QSqlQuery &query);

    DatabaseHandler *m_dbHandler;
};

#endif // SALESMANAGER_H
