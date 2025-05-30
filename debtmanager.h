#ifndef DEBTMANAGER_H
#define DEBTMANAGER_H

#include <QObject>
#include <QTableWidget>
#include <QDate>
#include <QSqlQuery>
#include "databasehandler.h"

class DebtManager : public QObject
{
    Q_OBJECT
public:
    explicit DebtManager(DatabaseHandler *dbHandler, QObject *parent = nullptr);

    bool loadDebtors(QTableWidget *tableWidget);
    void searchDebtors(QTableWidget *tableWidget, const QString &searchText);
    bool addDebtor(const QString &name, const QString &contact,
                   const QString &address, double debtAmount, const QDate &dateIncurred);
    bool removeDebtor(int debtorId);
    bool getDebtorStats(int &totalDebtors, double &totalDebt);

signals:
    void debtorsUpdated();

private:
    DatabaseHandler *m_dbHandler;

    // Helper methods to reduce code duplication
    bool populateTable(QTableWidget *tableWidget, const QString &sql);
    bool populateTableWithQuery(QTableWidget *tableWidget, QSqlQuery &query);
    bool executeQuery(const QString &sql, const QVariantList &params = {});
};

#endif // DEBTMANAGER_H
