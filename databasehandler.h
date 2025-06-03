#ifndef DATABASEHANDLER_H
#define DATABASEHANDLER_H

#include <QObject>
#include <QtSql>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlQueryModel>
class DatabaseHandler : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseHandler(QObject *parent = nullptr) : QObject(parent), m_loggedIn(false), m_isAdmin(false), m_userId(-1) {
        db = QSqlDatabase::addDatabase("QMYSQL");
        db.setHostName("localhost");
        db.setDatabaseName("utilisoft");
        db.setUserName("root");
        db.setPassword("");
    }

    ~DatabaseHandler() { if (db.isOpen()) db.close(); }

    bool connectToDatabase() {
        if (!db.open()) {
            qDebug() << "Database connection failed:" << db.lastError().text();
            return false;
        }
        qDebug() << "Database connected successfully";
        return true;
    }

    bool login(const QString &username, const QString &password) {
        if (!db.isOpen() && !connectToDatabase()) return false;

        QSqlQuery q;
        q.prepare("SELECT user_id, (role = 'Admin') as is_admin FROM Users WHERE username = ? AND password = ?");
        q.addBindValue(username);
        q.addBindValue(password);

        if (q.exec() && q.next()) {
            m_userId = q.value(0).toInt();
            m_isAdmin = q.value(1).toBool();
            m_loggedIn = true;
            emit loginStatusChanged(true, m_isAdmin);
            return true;
        }
        return false;
    }

    void logout() {
        if (db.isOpen() && m_userId != -1) { // Check if DB is open and user is valid
            QSqlQuery query(db); // Pass the database connection to the query
            query.prepare("UPDATE Users SET last_logout = NOW() WHERE user_id = :userId");
            query.bindValue(":userId", m_userId);
            if (!query.exec()) {
                qDebug() << "Logout query failed:" << query.lastError().text();
            }
        }
        m_loggedIn = m_isAdmin = false;
        m_userId = -1;
        emit loginStatusChanged(false, false);
    }

    bool executeQuery(const QString &queryString) { // Renamed parameter for clarity
        if (!db.isOpen()) {
            return false;
        }
        QSqlQuery query(db); // Pass the database connection
        if (!query.exec(queryString)) {
            qDebug() << "executeQuery failed:" << query.lastError().text();
            return false;
        }
        return true; // QSqlQuery::exec() returns true on success
    }

    // Getters
    bool isLoggedIn() const { return m_loggedIn; }
    bool isAdmin() const { return m_isAdmin; }
    int getCurrentUserId() const { return m_userId; }
    bool isConnected() const { return db.isOpen(); }
    QSqlDatabase getDatabase() const { return db; }
    bool m_isAdmin;
signals:
    void loginStatusChanged(bool loggedIn, bool isAdmin);

private:
    QSqlDatabase db;
    bool m_loggedIn;
    int m_userId;
};

#endif // DATABASEHANDLER_H
