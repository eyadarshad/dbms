#ifndef STOCKMANAGER_H
#define STOCKMANAGER_H

#include <QObject>
#include <QTableWidget>
#include "databasehandler.h"

class StockManager : public QObject
{
    Q_OBJECT

public:
    explicit StockManager(DatabaseHandler *dbHandler, QObject *parent = nullptr);

    bool loadStock(QTableWidget *tableWidget);
    void searchStock(QTableWidget *tableWidget, const QString &searchText);

signals:
    void stockUpdated();

private:
    DatabaseHandler *m_dbHandler;
};

#endif // STOCKMANAGER_H
