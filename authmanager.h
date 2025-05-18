#ifndef AUTHMANAGER_H
#define AUTHMANAGER_H

#include <QObject>
#include <QString>
#include "databasehandler.h"

class AuthManager : public QObject
{
    Q_OBJECT
public:
    explicit AuthManager(DatabaseHandler *dbHandler, QObject *parent = nullptr);

    bool login(const QString &username, const QString &password);
    void logout();
    bool isLoggedIn() const;
    bool isAdmin() const;
    int getCurrentUserId() const;

signals:
    void loginStatusChanged(bool loggedIn, bool isAdmin);

private:
    DatabaseHandler *m_dbHandler;
    bool m_loggedIn;
    bool m_isAdmin;
    int m_userId;
};

#endif // AUTHMANAGER_H
