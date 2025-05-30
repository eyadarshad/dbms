#pragma once
#ifndef SALESDASHBOARD_H
#define SALESDASHBOARD_H

#include <QWidget>
#include <QLineEdit>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QList>
#include <QSqlQuery>
#include "saleitem.h"

// Forward declarations
class DatabaseHandler;
class ProductManager;
class SalesManager;

class SalesDashboard : public QWidget
{
    Q_OBJECT

public:
    explicit SalesDashboard(DatabaseHandler *dbHandler, ProductManager *productManager,
                            SalesManager *salesManager, QWidget *parent = nullptr);
    ~SalesDashboard();

    void refreshData();
    void refreshProductList();
    void refreshSalesTable();

private slots:
    void onProductSearchTextChanged(const QString &text);
    void onSalesSearchTextChanged(const QString &text);
    void onProductSelected(int row, int column);
    void onProductSelectedFromWidget(int productId, QString productName, double price, QString category, int available);
    void onSelectedProductClicked(int row, int column);
    void onSellProductsClicked();
    void onClearSelectionClicked();
    void onQuantityChanged(bool increase);

private:
    void setupUI();
    void setupProductSearch();
    void setupDebugTables();
    void setupRightPanel(QVBoxLayout *rightLayout);
    void setupControls(QVBoxLayout *rightLayout);
    void connectSignals();
    void clearLayout(QLayout *layout);
    void resetSalesArea();
    void addProductToSelectedList(const SaleItem &item);
    void removeProduct(int row);
    void updateSelectedItemQuantity(int index, bool increase);
    void updateTotalAmount();
    void highlightSelectedProduct(int row);

    // Core components
    DatabaseHandler *m_dbHandler;
    ProductManager *m_productManager;
    SalesManager *m_salesManager;

    // Search widgets
    QLineEdit *m_productSearchEdit;
    QLineEdit *m_salesSearchEdit;

    // Tables
    QTableWidget *m_productsTable;
    QTableWidget *m_salesTable;
    QTableWidget *m_selectedProductsTable;

    // Layouts
    QVBoxLayout *m_recommendLayout;
    QVBoxLayout *m_selectedLayout;

    // Controls
    QPushButton *m_addQtyBtn;
    QPushButton *m_removeQtyBtn;
    QPushButton *m_sellProductsBtn;
    QPushButton *m_clearSelectionBtn;
    QLabel *m_totalLabel;

    // Data
    QList<SaleItem> m_selectedItems;
    double m_totalAmount;
    int m_currentSelectedRow;
};

#endif // SALESDASHBOARD_H
