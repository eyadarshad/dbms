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
    void on_pushButton_95_clicked();
    void on_loginbtn_clicked();
    void on_eyeButton_clicked();

private:
     Ui::MainWindow *ui;
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

    void setupIconManagement();
    void setupChart();
    void setupCalculator();
    void setupNavigation();
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
};

#endif

