#ifndef VENDORMANAGER_H
#define VENDORMANAGER_H

#include <QObject>
#include <QTableWidget>
#include <QDate>
#include "databasehandler.h"

class VendorManager : public QObject
{
    Q_OBJECT
public:
    explicit VendorManager(DatabaseHandler *dbHandler, QObject *parent = nullptr);

    // Load all vendors to the table widget
    bool loadVendors(QTableWidget *tableWidget);

    // Search vendors by name or address
    void searchVendors(QTableWidget *tableWidget, const QString &searchText);

    // Add a new vendor
    bool addVendor(const QString &name, const QString &address, const QString &contact,
                   double cashBalance, const QDate &dateOfSupply);

    // Remove a vendor
    bool removeVendor(int vendorId);

signals:
    void vendorsUpdated();

private:
    DatabaseHandler *m_dbHandler;
};

#endif // VENDORMANAGER_H
