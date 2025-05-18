#ifndef DATABASEHANDLER_H
#define DATABASEHANDLER_H

#include <QObject>
#include <QtSql>
#include <QMessageBox>
#include <QDebug>

class DatabaseHandler : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseHandler(QObject *parent = nullptr);
    ~DatabaseHandler();

    // Connection management
    bool connectToDatabase();
    void closeConnection();
    bool isConnected() const;

    // Authentication
    bool validateLogin(const QString &username, const QString &password, bool &isAdmin);

    // Database execution helper
    bool executeQuery(const QString &queryStr);

    // Direct access to database for managers
    QSqlDatabase getDatabase() const { return db; }

private:
    QSqlDatabase db;
};

#endif // DATABASEHANDLER_H
