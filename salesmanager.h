#ifndef SALESMANAGER_H
#define SALESMANAGER_H


#include <QObject>
#include <QTableWidget>
#include <QList>

class QWidget;
class DatabaseHandler;

// Represents an individual item in a sale transaction
struct SaleItem {
    int productId;
    QString productName;
    double unitPrice;
    QString category;
    int quantity;
    int available;
    double totalPrice;
};

class SalesManager : public QObject
{
    Q_OBJECT

public:
    explicit SalesManager(DatabaseHandler *dbHandler, QObject *parent = nullptr);

    bool loadSales(QTableWidget *tableWidget);
    void searchSales(QTableWidget *tableWidget, const QString &searchText);
    bool searchProducts(QTableWidget *tableWidget, const QString &searchText);
    bool getProductInfo(int productId, SaleItem &item);
    bool getProductsForRecommendation(QWidget *parentWidget, const QString &searchText);
    bool processSale(const QList<SaleItem> &items, int userId);
    bool getSalesStats(int &totalSales, double &totalAmount, double &profitMargin);

signals:
    void salesUpdated();

private:
    DatabaseHandler *m_dbHandler;
};

#endif // SALESMANAGER_H
