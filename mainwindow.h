#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCharts>
#include <QChartView>
#include <QLineSeries>
#include <QPieSeries>
#include <QPieSlice>
#include <QtSql/QtSql>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <QMap>
#include <QPair>
#include <QBrush>
#include <QDir>
#include <QFileInfoList>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QAbstractAnimation>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QChart>
#include <QLineSeries>
#include <QJSEngine>
#include <QJSValue>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDate>
#include <tuple>

#include "databasehandler.h"
#include "debtmanager.h"
#include "productmanager.h"
#include "vendormanager.h"
#include "workermanager.h"
#include "salesmanager.h"
#include "salesdashboard.h"
#include "stockmanager.h"
#include "saleitem.h"// This includes the SaleItem structure

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void enableLightMode();
    void enableDarkMode();
    void on_pushButton_clicked();
    void on_loginbtn_clicked();
    void loginpage();
    void on_eyeButton_clicked();

    // Slots for debtor management
    void on_addDebtorBtn_clicked();   // For opening the add debtor form
    void on_addDebtorBtn_2_clicked(); // For submitting the debtor form
    void on_removeDebtorBtn_clicked(); // For removing a debtor
    void on_debtorSearchEdit_textChanged(const QString &arg1); // For search functionality

    // Slots for product management
    void on_addProductBtn_clicked();   // For opening the add product form
    void on_addProductBtn_2_clicked(); // For submitting the product form
    void on_removeProductBtn_clicked(); // For removing a product
    void on_productSearchEdit_textChanged(const QString &arg1); // For search functionality

    // Slots for vendor management
    void on_addVendorBtn_clicked();    // For opening the add vendor form
    void on_addVendorBtn_2_clicked();  // For submitting the vendor form
    void on_removeVendorBtn_clicked(); // For removing a vendor
    void on_vendorSearchEdit_textChanged(const QString &arg1); // For search functionality

    // Slots for worker management
    void on_addWorkerBtn_clicked();    // For opening the add worker form
    void on_addWorkerBtn_2_clicked();  // For submitting the worker form
    void on_removeWorkerBtn_clicked(); // For removing a worker
    void on_workerSearchEdit_textChanged(const QString &arg1); // For search functionality

    // Sales dashboard slots
    void on_productSalesSearchEdit_textChanged(const QString &text);
    void on_salesSearchEdit_textChanged(const QString &text);
    void on_searchProductTable_cellClicked(int row, int column);
    void on_selectedProductsTable_cellClicked(int row, int column);
    void on_addQtyBtn_clicked();
    void on_removeQtyBtn_clicked();
    void on_sellProductsBtn_clicked();
    void on_clearSelectionBtn_clicked(); // Handle selected product removal
    void on_workerStockSearchEdit_textChanged(const QString &arg1);
    void on_workerProductSearchEdit_textChanged(const QString &text);// Dashboard update
    void updateDashboard(); // Update the dashboard stats
    void on_addProductBtn_4_clicked();
    // Handle data updates
    void onDebtorsUpdated();
    void onProductsUpdated();
    void onVendorsUpdated();
    void onWorkersUpdated();
    void onSalesUpdated(); // Added sales update handler

    void on_viewStockSearchEdit_textChanged(const QString &arg1);

    void on_cross_2_clicked();

private:
    Ui::MainWindow *ui;

    DatabaseHandler *m_dbHandler;
    DebtManager *m_debtManager;
    ProductManager *m_productManager;
    VendorManager *m_vendorManager;
    WorkerManager *m_workManager;
    SalesManager *m_salesManager;
    SalesDashboard *m_salesdashboard;
    StockManager *m_stockManager;

    // Structure for storing selected products in sales


    struct TableSettings {
        QMap<int, QBrush> headerBackgrounds;
        QMap<int, QBrush> headerForegrounds;
        QMap<QPair<int, int>, QBrush> itemBackgrounds;
        QMap<QPair<int, int>, QBrush> itemForegrounds;
    };

    bool isDarkMode;
    bool passwordVisible;
    QSqlDatabase DBConnection;
    QMap<QString, QString> iconPairMappings;
    QMap<QString, QString> colorMappings;
    QMap<QWidget*, QString> originalStylesheets;
    QString originalMainStylesheet;
    QMap<QTableWidget*, TableSettings> originalTableSettings;
    void setupSalesTable(QTableWidget *table, const QStringList &headers, int columnCount);
    // Setup methods
    void setupIconManagement();
    void setupChart();
    void setupCalculator();
    void setupNavigation();
    void setupDebtManager();
    void setupProductManager();
    void setupVendorManager();
    void setupWorkerManager(); // Added setup function for sales management
    void setupStockManager();


    // Helper methods
    void connectPageButton(QPushButton *button, int index);
    void showDarkMessageBox(const QString &title, const QString &message);
    QString getAlternateThemePath(const QString &currentPath);
    void updateAllIcons();
    void initializeColorMappings();
    QColor stringToColor(const QString &colorStr);
    QString colorToString(const QColor &color, const QString &originalFormat);
    QString applyLightModeColor(const QString &originalColor);
    QString processStyleSheetForLightMode(const QString &styleSheet);
    void applyLightModeToTable(QTableWidget* table);
    void applyLightModeToAllWidgets();
    void restoreDarkModeToAllWidgets();
    void logoutUser();

    // Table setup and refresh methods
    void setupTableWidget(QTableWidget *table, const QStringList &headers);
    void refreshDebtorTable();
    void refreshProductTable();
    void refreshVendorTable();
    void refreshWorkerTable();// Added helper to refresh selected products
    void refreshStockTable();

    // Form data retrieval methods
    std::tuple<QString, QString, QString, double, QDate> getDebtorFormData();
    std::tuple<QString, double, QString, int, QDate> getProductFormData();

    // Input validation methods
    bool validateDebtorInput(const QString &name, const QString &contact,
                             const QString &address, double amount);
    bool validateProductInput(const QString &name, double price, int quantity);

    // Form clearing methods
    void clearDebtorForm();
    void clearProductForm();

    // User interaction methods
    bool confirmRemoval(const QString &type, const QString &name);
    void showSuccess(const QString &message);
    void showError(const QString &message);
    void showWarning(const QString &message);
    bool showSuccessWithOk(const QString &message);

    // Sales specific methods
    void clearForm(const QList<QLineEdit*> &fields);
    std::tuple<QString, QString, QString, QString> getFormData(const QList<QLineEdit*> &fields);
    bool validateInput(const QStringList &fields);
    std::optional<int> getSelectedId(QTableWidget *table, const QString &type);
    void setupTableHeaders(QTableWidget *table, const QStringList &headers);// Added helper function to add product to selection

    // Stock management event handler
    void onStockUpdated();

    QList<SaleItem> m_selectedItems;
    double m_totalAmount;
    int m_currentSelectedRow;

    void refreshSalesTable();
    void refreshProductSalesTable();
    void refreshSelectedProductsTable();
    void updateSalesTotals();
    void addProductToSelection(const SaleItem &item);
    void highlightSelectedProduct(int row);
    void connectSalesSignals();

    void setupSalesManager();
    void integrateSalesDashboard();
    bool initializeSalesSystem();
    void showSalesDashboard();


};

#endif // MAINWINDOW_H
