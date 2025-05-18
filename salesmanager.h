#ifndef SALESMANAGER_H
#define SALESMANAGER_H

#include <QObject>
#include <QTableWidget>
#include <QList>
#include <QPair>
#include <QDate>
#include "databasehandler.h"

struct SaleItem {
    int productId;
    QString productName;
    double unitPrice;
    int quantity;
    double totalPrice;
};

class SalesManager : public QObject
{
    Q_OBJECT
public:
    explicit SalesManager(DatabaseHandler *dbHandler, QObject *parent = nullptr);

    // Load all sales to the table widget
    bool loadSales(QTableWidget *tableWidget);

    // Search sales by various criteria
    void searchSales(QTableWidget *tableWidget, const QString &searchText);

    // Process a sale transaction
    bool processSale(const QList<SaleItem> &items, int userId, int debtorId = -1);

    // Get dashboard stats
    bool getSalesStats(int &totalSales, double &totalAmount, double &profitMargin);

signals:
    void salesUpdated();

private:
    DatabaseHandler *m_dbHandler;
};

#endif // SALESMANAGER_H
