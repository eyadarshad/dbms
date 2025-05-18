#ifndef SALESDASHBOARD_H
#define SALESDASHBOARD_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QList>
#include "databasehandler.h"
#include "salesmanager.h"
#include "productmanager.h"
#include "authmanager.h"

class SalesDashboard : public QWidget
{
    Q_OBJECT

public:
    explicit SalesDashboard(DatabaseHandler *dbHandler,
                            ProductManager *productManager,
                            SalesManager *salesManager,
                            AuthManager *authManager,
                            QWidget *parent = nullptr);
    ~SalesDashboard();

    void setupUI();
    void refreshProductList();
    void refreshSelectedProducts();
    void refreshSalesTable();

public slots:
    void onProductSearchTextChanged(const QString &text);
    void onSalesSearchTextChanged(const QString &text);
    void onProductSelected(int row, int column);
    void onProductRemoved();
    void onQuantityChanged(bool increase);
    void onSellProductsClicked();
    void onClearSelectionClicked();
    void updateTotals();

private:
    // Database and manager objects
    DatabaseHandler *m_dbHandler;
    ProductManager *m_productManager;
    SalesManager *m_salesManager;
    AuthManager *m_authManager;

    // UI elements
    QTableWidget *m_productsTable;
    QTableWidget *m_selectedProductsTable;
    QTableWidget *m_salesTable;
    QLineEdit *m_productSearchEdit;
    QLineEdit *m_salesSearchEdit;
    QPushButton *m_sellProductsBtn;
    QPushButton *m_clearSelectionBtn;
    QLabel *m_totalLabel;
    QPushButton *m_addQtyBtn;
    QPushButton *m_removeQtyBtn;

    // Store selected products
    QList<SaleItem> m_selectedItems;
    double m_totalAmount;

    void setupProductSearch();
    void setupProductsTable();
    void setupSelectedProductsTable();
    void setupSalesTable();
    void setupActionButtons();
};
#endif // SALESDASHBOARD_H
