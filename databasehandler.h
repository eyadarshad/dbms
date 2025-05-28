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
        db.exec("UPDATE Users SET last_logout = NOW() WHERE user_id = " + QString::number(m_userId));
        m_loggedIn = m_isAdmin = false;
        m_userId = -1;
        emit loginStatusChanged(false, false);
    }

    bool executeQuery(const QString &q) {
        return db.isOpen() ? db.exec(q).isActive() : false;
    }

    // Getters
    bool isLoggedIn() const { return m_loggedIn; }
    bool isAdmin() const { return m_isAdmin; }
    int getCurrentUserId() const { return m_userId; }
    bool isConnected() const { return db.isOpen(); }
    QSqlDatabase getDatabase() const { return db; }

signals:
    void loginStatusChanged(bool loggedIn, bool isAdmin);

private:
    QSqlDatabase db;
    bool m_loggedIn, m_isAdmin;
    int m_userId;
};

#endif // DATABASEHANDLER_H
