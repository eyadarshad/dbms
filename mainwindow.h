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

#include "databasehandler.h"
#include "authmanager.h"
#include "debtmanager.h"
#include "productmanager.h"
#include "vendormanager.h"
#include "workermanager.h"
#include "salesmanager.h"
#include "clickableWidget.h"
#include "salesdashboard.h"  // This includes the SaleItem structure

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
    void on_productSalesSearchEdit_textChanged(const QString &arg1); // For product search in sales
    void on_salesSearchEdit_textChanged(const QString &arg1); // For sales table search
    void on_addQtyBtn_clicked(); // Increase quantity of selected product
    void on_removeQtyBtn_clicked(); // Decrease quantity of selected product
    void on_sellProductsBtn_clicked(); // Process the sale
    void on_clearSelectionBtn_clicked(); // Clear selected products
    void on_searchProductTable_cellClicked(int row, int column); // Handle product selection
    void on_selectedProductsTable_cellClicked(int row, int column); // Handle selected product removal

    // Dashboard update
    void updateDashboard(); // Update the dashboard stats

    // Handle data updates
    void onDebtorsUpdated();
    void onProductsUpdated();
    void onVendorsUpdated();
    void onWorkersUpdated();
    void onSalesUpdated(); // Added sales update handler

private:
    Ui::MainWindow *ui;

    DatabaseHandler *m_dbHandler;
    AuthManager *m_authManager;
    DebtManager *m_debtManager;
    ProductManager *m_productManager;
    VendorManager *m_vendorManager;
    WorkerManager *m_workManager;
    SalesManager *m_salesManager;
    SalesDashboard *m_salesdashboard;

    // Structure for storing selected products in sales
    QList<SaleItem> m_selectedItems;
    double m_totalAmount;

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

    // Setup methods
    void setupIconManagement();
    void setupChart();
    void setupCalculator();
    void setupNavigation();
    void setupDebtManager();
    void setupProductManager();
    void setupVendorManager();
    void setupWorkerManager();
    void setupSalesManager();  // Added setup function for sales management

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

    // Table refresh methods
    void refreshDebtorTable();
    void refreshProductTable();
    void refreshVendorTable();
    void refreshWorkerTable();
    void refreshSalesTable();  // Added helper function to refresh sales table
    void refreshProductSalesTable(); // Added helper to refresh products for sales
    void refreshSelectedProductsTable(); // Added helper to refresh selected products

    // Sales specific methods
    void updateSalesTotals(); // Added helper to update sales totals
    void addProductToSelection(const SaleItem &item); // Added helper function to add product to selection
};

#endif // MAINWINDOW_H
