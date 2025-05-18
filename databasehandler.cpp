#include "databasehandler.h"

DatabaseHandler::DatabaseHandler(QObject *parent)
    : QObject(parent)
{
    // ifqDebug() << "SQL Error: " << lastError();
}

DatabaseHandler::~DatabaseHandler()
{
    closeConnection();
}

bool DatabaseHandler::connectToDatabase()
{
    // Setup database connection
    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("localhost");
    db.setDatabaseName("utilisoft");
    db.setUserName("root");  // Default MySQL username, adjust as needed
    db.setPassword("");      // Default empty password, adjust as needed

    if (!db.open()) {
        qDebug() << "Database connection failed:" << db.lastError().text();
        return false;
    }

    qDebug() << "Database connected successfully";
    return true;
}

void DatabaseHandler::closeConnection()
{
    if (db.isOpen()) {
        db.close();
        qDebug() << "Database connection closed";
    }
}

bool DatabaseHandler::isConnected() const
{
    return db.isOpen();
}

bool DatabaseHandler::validateLogin(const QString &username, const QString &password, bool &isAdmin)
{
    if (!isConnected()) {
        qDebug() << "Database not connected";
        return false;
    }

    QSqlQuery query;
    query.prepare("SELECT user_id, role FROM Users WHERE username = :username AND password = :password");
    query.bindValue(":username", username);
    query.bindValue(":password", password);  // In a real app, passwords should be hashed

    if (!query.exec()) {
        qDebug() << "Login query failed:" << query.lastError().text();
        return false;
    }

    if (query.next()) {
        // User found, check role
        isAdmin = (query.value("role").toString() == "Admin");
        return true;
    }

    return false;
}

bool DatabaseHandler::executeQuery(const QString &queryStr)
{
    if (!isConnected()) {
        qDebug() << "Database not connected";
        return false;
    }

    QSqlQuery query;
    if (!query.exec(queryStr)) {
        qDebug() << "Query execution failed:" << query.lastError().text();
        return false;
    }

    return true;
}
