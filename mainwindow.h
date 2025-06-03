#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCharts>
#include <QChartView>
#include <QLineSeries>
#include <QDateTimeAxis>
#include <QValueAxis>
#include <QtSql>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <QMap>
#include <QPair>
#include <QBrush>
#include <QDate>
#include <QJSEngine>
#include <QPropertyAnimation>
#include <optional>
#include <QtCharts>

namespace Ui { class MainWindow; }
class DatabaseHandler;
class DebtManager;
class ProductManager;
class VendorManager;
class WorkerManager;
class SalesManager;
class SalesDashboard;
class StockManager;
struct SaleItem;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void enableLightMode();
    void enableDarkMode();
    void on_eyeButton_clicked();
    void on_loginbtn_clicked();
    void loginpage();
    void on_pushButton_clicked();

    void on_addDebtorBtn_clicked();
    void on_addDebtorBtn_2_clicked();
    void on_removeDebtorBtn_clicked();
    void on_debtorSearchEdit_textChanged(const QString &searchText);
    void onDebtorsUpdated();

    void on_addProductBtn_clicked();
    void on_addProductBtn_4_clicked();
    void on_addProductBtn_2_clicked();
    void on_removeProductBtn_clicked();
    void on_productSearchEdit_textChanged(const QString &searchText);
    void on_workerProductSearchEdit_textChanged(const QString &searchText);
    void onProductsUpdated();

    void on_addVendorBtn_clicked();
    void on_addVendorBtn_2_clicked();
    void on_removeVendorBtn_clicked();
    void on_vendorSearchEdit_textChanged(const QString &searchText);
    void onVendorsUpdated();

    void on_addWorkerBtn_clicked();
    void on_addWorkerBtn_2_clicked();
    void on_removeWorkerBtn_clicked();
    void on_workerSearchEdit_textChanged(const QString &searchText);
    void onWorkersUpdated();

    void on_viewStockSearchEdit_textChanged(const QString &searchText);
    void on_workerStockSearchEdit_textChanged(const QString &searchText);
    void onStockUpdated();

    void on_productSalesSearchEdit_2_textChanged(const QString &searchText);
    void on_salesSearchEdit_textChanged(const QString &searchText);
    void on_searchProductTable_cellClicked(int row, int column);
    void on_selectedProductsTable_cellClicked(int row, int column);
    void on_addQtyBtn_clicked();
    void on_removeQtyBtn_clicked();
    void on_sellProductsBtn_clicked();
    void on_clearSelectionBtn_clicked();
    void onSalesUpdated();

    void updateDashboard();
    void on_cross_2_clicked();

private:
    void setupChart();
    void updateSalesChart();
    void setupCalculator();
    void setupNavigation();
    void connectPageButton(QPushButton *button, int pageIndex);

    void setupDebtManager();
    void setupProductManager();
    void setupVendorManager();
    void setupWorkerManager();
    void setupStockManager();
    void setupSalesManager();
    void integrateSalesDashboard();
    bool initializeSalesSystem();

    void initializeColorMappings();
    QString processStyleSheetForLightMode(const QString &styleSheet);
    QString applyLightModeColor(const QString &originalColor);
    QColor stringToColor(const QString &colorStr);
    QString colorToString(const QColor &color, const QString &originalFormat);
    void applyLightModeToTable(QTableWidget* table);
    void applyLightModeToAllWidgets();
    void restoreDarkModeToAllWidgets();

    void logoutUser();

    void setupTableWidget(QTableWidget *table, const QStringList &headers);
    void setupTableHeaders(QTableWidget *table, const QStringList &headers);
    void setupSalesTable(QTableWidget *table, const QStringList &headers, int columnCount);

    void refreshDebtorTable();
    void refreshProductTable();
    void refreshVendorTable();
    void refreshWorkerTable();
    void refreshStockTable();

    std::tuple<QString, QString, QString, double, QDate> getDebtorFormData();
    std::tuple<QString, double, QString, int, QDate> getProductFormData();
    bool validateDebtorInput(const QString &name, const QString &contact, const QString &address, double amount);
    bool validateProductInput(const QString &name, double price, int quantity);
    void clearDebtorForm();
    void clearProductForm();

    void clearForm(const QList<QLineEdit*> &fields);
    std::tuple<QString, QString, QString, QString> getFormData(const QList<QLineEdit*> &fields);
    bool validateInput(const QStringList &fields);
    std::optional<int> getSelectedId(QTableWidget *table, const QString &type);

    void showDarkMessageBox(const QString &title, const QString &message);
    bool confirmRemoval(const QString &type, const QString &name);
    void showSuccess(const QString &message);
    void showError(const QString &message);
    void showWarning(const QString &message);
    bool showSuccessWithOk(const QString &message);

    void refreshSalesTable();
    void refreshProductSalesTable();
    void refreshSelectedProductsTable();
    void updateSalesTotals();
    void addProductToSelection(const SaleItem &item);
    void highlightSelectedProduct(int rowInSelectedTable);

    Ui::MainWindow *ui;
    DatabaseHandler *m_dbHandler;
    DebtManager *m_debtManager;
    ProductManager *m_productManager;
    VendorManager *m_vendorManager;
    WorkerManager *m_workManager;
    StockManager *m_stockManager;
    SalesManager *m_salesManager;
    SalesDashboard *m_salesdashboard;

    bool isDarkMode;
    bool passwordVisible;

    QMap<QString, QString> colorMappings;
    QMap<QWidget*, QString> originalStylesheets;
    QString originalMainStylesheet;

    struct TableSettings {
        QMap<int, QBrush> headerBackgrounds;
        QMap<int, QBrush> headerForegrounds;
        QMap<QPair<int, int>, QBrush> itemBackgrounds;
        QMap<QPair<int, int>, QBrush> itemForegrounds;
    };
    QMap<QTableWidget*, TableSettings> originalTableSettings;

    QList<SaleItem> m_selectedItems;
    double m_totalAmount;
    int m_currentSelectedRow;

    QChart *m_salesChart;
    QLineSeries *m_salesSeries;
    QChartView *m_chartView;
    void connectSalesSignals();
    void showSalesDashboard();

};

#endif // MAINWINDOW_H
