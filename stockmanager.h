#ifndef STOCKMANAGER_H
#define STOCKMANAGER_H

#include <QObject>
#include <QTableWidget>
#include <QDate>
#include "databasehandler.h"

class StockManager : public QObject
{
    Q_OBJECT
public:
    explicit StockManager(DatabaseHandler *dbHandler, QObject *parent = nullptr);

    // Load all stock items to the table widget
    bool loadStockItems(QTableWidget *tableWidget);

    // Search stock items by name or category
    void searchStockItems(QTableWidget *tableWidget, const QString &searchText);

    // Get dashboard stats for stock
    bool getStockStats(int &totalStock, int &lowStockCount);

signals:
    void stockUpdated();

private:
    DatabaseHandler *m_dbHandler;

    // Helper method to calculate remaining quantity
    int calculateRemainingQuantity(int productId, int totalQuantity);
};

#endif // STOCKMANAGER_H
