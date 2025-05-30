#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include<QtCharts>
#include<QChartView>
#include<QLineSeries>
#include<QPieSeries>
#include<QPieSlice>
#include <QtSql/QtSql>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <QMap>
#include <QPair>
#include <QBrush>

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
    void on_pushButton_95_clicked();

    void on_loginbtn_clicked();

    void on_pushButton_clicked();

    void on_eyeButton_clicked();
    void enableLightMode();
    void enableDarkMode();

private:
    Ui::MainWindow *ui;
    void setupChart();
    void setupCalculator();
    void connectPageButton(QPushButton* button, int index); // match .cpp file!
    void setupNavigation();
    void applyLightMode();
    void applyDarkMode();
    void connectThemeToggles();
    QSqlDatabase DBConnection;
    void showDarkMessageBox(const QString &title, const QString &message);
    struct TableSettings {
        QMap<int, QBrush> headerBackgrounds;
        QMap<int, QBrush> headerForegrounds;
        QMap<QPair<int, int>, QBrush> itemBackgrounds;
        QMap<QPair<int, int>, QBrush> itemForegrounds;
    };
    bool isDarkMode;

    // Store original stylesheets for restoration
    QMap<QWidget*, QString> originalStylesheets;
    QString originalMainStylesheet;
    QString sidebarStyleSheet;

    // Store original table settings
    QMap<QTableWidget*, TableSettings> originalTableSettings;

    // Color mapping for light mode
    QMap<QString, QString> colorMappings;

    // Helper methods
    void applyLightModeToAllWidgets();
    void restoreDarkMode();
    void applyLightModeToTable(QTableWidget* table);
    void initializeColorMappings();

    QColor invertColor(const QColor &color);
    QColor stringToColor(const QString &colorStr);
    QString colorToString(const QColor &color, const QString &originalFormat);
    QString extractColor(const QString &styleSheet, const QString &property);
    QString replaceColor(const QString &styleSheet, const QString &property, const QString &newColor);
    QString processStyleSheetForLightMode(const QString &styleSheet);
    QString applyLightModeColor(const QString &originalColor);
    void applyLightModeEffects();
};

#endif // MAINWINDOW_H
