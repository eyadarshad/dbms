#include "authmanager.h"

AuthManager::AuthManager(DatabaseHandler *dbHandler, QObject *parent)
    : QObject(parent)
    , m_dbHandler(dbHandler)
    , m_loggedIn(false)
    , m_isAdmin(false)
    , m_userId(-1)
{
}

bool AuthManager::login(const QString &username, const QString &password)
{
    bool isAdmin = false;
    // Try to validate credentials through database handler
    if (m_dbHandler->validateLogin(username, password, isAdmin)) {
        // Login successful, update state
        m_loggedIn = true;
        m_isAdmin = isAdmin;

        // Get user ID from database (assuming validateLogin stored it internally)
        // For a more robust implementation, this would be returned by validateLogin
        // but for simplicity we'll assume user ID is stored elsewhere
        m_userId = 1; // This should come from the database in a real implementation

        // Emit signal to notify UI of login status change
        emit loginStatusChanged(m_loggedIn, m_isAdmin);
        return true;
    }
    return false;
}

void AuthManager::logout()
{
    m_loggedIn = false;
    m_isAdmin = false;
    m_userId = -1;
    emit loginStatusChanged(m_loggedIn, m_isAdmin);
}

bool AuthManager::isLoggedIn() const
{
    return m_loggedIn;
}

bool AuthManager::isAdmin() const
{
    return m_isAdmin;
}

int AuthManager::getCurrentUserId() const
{
    return m_userId;
}
