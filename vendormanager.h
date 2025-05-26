#ifndef VENDORMANAGER_H
#define VENDORMANAGER_H

#include <QObject>
#include <QTableWidget>
#include <QDate>
#include <QVariantList>
#include "databasehandler.h"

class VendorManager : public QObject
{
    Q_OBJECT
public:
    explicit VendorManager(DatabaseHandler *dbHandler, QObject *parent = nullptr);

    bool loadVendors(QTableWidget *tableWidget);
    void searchVendors(QTableWidget *tableWidget, const QString &searchText);
    bool addVendor(const QString &name, const QString &address, const QString &contact,
                   double cashBalance, const QDate &dateOfSupply);
    bool removeVendor(int vendorId);

signals:
    void vendorsUpdated();

private:
    DatabaseHandler *m_dbHandler;

    // Helper methods to reduce code duplication
    bool populateTable(QTableWidget *table, const QString &queryStr);
    bool populateTableWithQuery(QTableWidget *table, QSqlQuery &query);
    bool executeUpdate(const QString &queryStr, const QVariantList &params = {});
};

#endif // VENDORMANAGER_H
