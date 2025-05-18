#ifndef DEBTMANAGER_H
#define DEBTMANAGER_H

#include <QObject>
#include <QTableWidget>
#include <QDate>
#include "databasehandler.h"

class DebtManager : public QObject
{
    Q_OBJECT
public:
    explicit DebtManager(DatabaseHandler *dbHandler, QObject *parent = nullptr);

    // Load all debtors to the table widget
    bool loadDebtors(QTableWidget *tableWidget);

    // Search debtors by name
    void searchDebtors(QTableWidget *tableWidget, const QString &searchText);

    // Add a new debtor
    bool addDebtor(const QString &name, const QString &contact, const QString &email,
                   const QString &address, double debtAmount, const QDate &dateIncurred);

    // Remove a debtor (when debt is cleared)
    bool removeDebtor(int debtorId);

    // Get dashboard stats
    bool getDebtorStats(int &totalDebtors, double &totalDebt);

signals:
    void debtorsUpdated();

private:
    DatabaseHandler *m_dbHandler;
};

#endif // DEBTMANAGER_H
