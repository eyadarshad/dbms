#ifndef SALESDASHBOARD_H
#define SALESDASHBOARD_H

#include <QWidget>
#include <QLineEdit>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <QList>

class DatabaseHandler;
class ProductManager;
class SalesManager;
class AuthManager;
class QVBoxLayout;

// Forward declaration of SaleItem from salesmanager.h
struct SaleItem;

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

    void refreshProductList();
    void refreshSalesTable();
    void resetSalesArea();

private slots:
    void onProductSearchTextChanged(const QString &text);
    void onSalesSearchTextChanged(const QString &text);
    void onProductSelected(int row, int column);
    void onSelectedProductClicked(int row, int column);
    void onQuantityChanged(bool increase);
    void onRemoveSelectedProduct();
    void onSellProductsClicked();
    void onClearSelectionClicked();

private:
    void setupUI();
    void setupProductSearch();
    void setupProductsTable();
    void setupSelectedProductsTable();
    void setupSalesTable();
    void setupActionButtons();
    void updateTotalAmount();
    void highlightSelectedProduct(int row);
    void addProductToSelectedList(const SaleItem &item);
    void updateSelectedItemQuantity(int index, bool increase);

private:
    DatabaseHandler *m_dbHandler;
    ProductManager *m_productManager;
    SalesManager *m_salesManager;
    AuthManager *m_authManager;

    // UI Elements
    QLineEdit *m_productSearchEdit;
    QTableWidget *m_productsTable;
    QLineEdit *m_salesSearchEdit;
    QTableWidget *m_salesTable;
    QTableWidget *m_selectedProductsTable;
    QLabel *m_totalLabel;
    QPushButton *m_addQtyBtn;
    QPushButton *m_removeQtyBtn;
    QPushButton *m_clearSelectionBtn;
    QPushButton *m_sellProductsBtn;
    QVBoxLayout *m_recommendLayout;
    QVBoxLayout *m_selectedLayout;

    // Data
    QList<SaleItem> m_selectedItems;
    double m_totalAmount;
    int m_currentSelectedRow;
};

#endif // SALESDASHBOARD_H
