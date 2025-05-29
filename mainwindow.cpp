#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "databasehandler.h"
#include "debtmanager.h"
#include "productmanager.h"
#include "vendormanager.h"
#include "workermanager.h"
#include "stockmanager.h"
#include "salesmanager.h"
#include "salesdashboard.h"
#include "saleitem.h"

#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>
#include <QColor>
#include <QRegularExpression>
#include <QStyleFactory>
#include <QDateTime>
#include <QVBoxLayout>
#include <QJSEngine>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <optional>
#include <QtCharts>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_dbHandler(new DatabaseHandler(this))
    , m_debtManager(nullptr)
    , m_productManager(nullptr)
    , m_vendorManager(nullptr)
    , m_workManager(nullptr)
    , m_stockManager(nullptr)
    , m_salesManager(nullptr)
    , m_salesdashboard(nullptr)
    , isDarkMode(true)
    , passwordVisible(false)
    , m_totalAmount(0.0)
    , m_currentSelectedRow(-1)
    , m_salesChart(nullptr)
    , m_salesSeries(nullptr)
    , m_chartView(nullptr)
{
    ui->setupUi(this);
    loginpage();

    QApplication::setStyle(QStyleFactory::create("Fusion"));
    qApp->setStyleSheet("QLabel { color: #ffffff; }"); // Default to dark mode text color

    QList<QPushButton*> lightModeButtons = {
        ui->pushButton_83, ui->pushButton_47, ui->pushButton_73, ui->pushButton_65,
        ui->pushButton_55, ui->pushButton_68, ui->pushButton_43, ui->pushButton_29,
        ui->pushButton_44, ui->pushButton_93, ui->pushButton_109
    };
    QList<QPushButton*> darkModeButtons = {
        ui->pushButton_84, ui->pushButton_48, ui->pushButton_74, ui->pushButton_66,
        ui->pushButton_56, ui->pushButton_67, ui->pushButton_106, ui->pushButton_100,
        ui->pushButton_108, ui->pushButton_94, ui->pushButton_110
    };

    for (QPushButton* btn : lightModeButtons) {
        if(btn) connect(btn, &QPushButton::clicked, this, &MainWindow::enableLightMode);
    }
    for (QPushButton* btn : darkModeButtons) {
        if(btn) connect(btn, &QPushButton::clicked, this, &MainWindow::enableDarkMode);
    }

    if(ui->eyeButton) ui->eyeButton->setIcon(QIcon(":/new/prefix1/images/view-Stroke-Rounded.png"));
    if(ui->password_login) ui->password_login->setEchoMode(QLineEdit::Password);

    if (!m_dbHandler->connectToDatabase()) {
        QMessageBox::critical(this, "Database Error", "Failed to connect to database. Please check your connection.");
        // Consider disabling UI elements or exiting if DB connection is critical
    }

    setupDebtManager();
    setupProductManager(); // ProductManager is needed by SalesDashboard
    setupVendorManager();
    setupWorkerManager();
    setupStockManager();
    initializeSalesSystem(); // This creates SalesDashboard and SalesManager

    setupChart(); // Sets up the sales chart
    setupCalculator();
    setupNavigation(); // This should be called AFTER SalesDashboard is potentially added to stackedWidget
    updateDashboard(); // Initial dashboard update
}

MainWindow::~MainWindow() {
    delete ui;
    // m_dbHandler is a child of MainWindow, Qt handles its deletion.
    // Manager pointers (m_debtManager, etc.) are also children if `this` is passed as parent.
}

void MainWindow::loginpage(){
    if(ui->stackedWidget) ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::initializeColorMappings() {
    colorMappings.clear();
    colorMappings["#0a0a0a"] = "#ffffff";
    colorMappings["#121212"] = "#f8fafc";
    colorMappings["#181818"] = "#f1f5f9";
    colorMappings["#262626"] = "#e2e8f0";
    colorMappings["#3a3a3a"] = "#cbd5e1";
    colorMappings["#ffffff"] = "#0a0a0a"; // For reversing light to dark
    colorMappings["#565656"] = "#94a3b8";
    colorMappings["#adadad"] = "#64748b";
    colorMappings["#626262"] = "#475569";
    colorMappings["#fcfcfc"] = "#1e293b";
    colorMappings["white"] = "#0f172a";     // Dark text for light mode
    colorMappings["#e0e4e4"] = "#334155";
    colorMappings["#494949"] = "#1e293b";
    colorMappings["#436cfd"] = "#2563eb";
    colorMappings["#0078d7"] = "#3b82f6";
    colorMappings["#4fc3f7"] = "#0ea5e9";
    colorMappings["#66bb6a"] = "#10b981";
    colorMappings["#ff7043"] = "#f59e0b";
    colorMappings["#ef5350"] = "#ef4444";
}

QColor MainWindow::stringToColor(const QString &colorStr) {
    if (colorStr.startsWith("#")) {
        return QColor(colorStr);
    } else if (colorStr.startsWith("rgb")) {
        QString cleaned = colorStr;
        cleaned.remove("rgb(").remove("rgba(").remove(")").remove(" ");
        QStringList parts = cleaned.split(',');
        if (parts.size() >= 3) {
            int r = parts[0].toInt();
            int g = parts[1].toInt();
            int b = parts[2].toInt();
            int a = (parts.size() > 3) ? parts[3].toInt() : 255;
            return QColor(r, g, b, a);
        }
    }
    return QColor(colorStr); // Fallback
}

QString MainWindow::colorToString(const QColor &color, const QString &originalFormat) {
    if (originalFormat.startsWith("#")) return color.name(QColor::HexRgb);
    if (originalFormat.startsWith("rgba(")) return QString("rgba(%1,%2,%3,%4)").arg(color.red()).arg(color.green()).arg(color.blue()).arg(color.alpha());
    if (originalFormat.startsWith("rgb(")) return QString("rgb(%1,%2,%3)").arg(color.red()).arg(color.green()).arg(color.blue());
    return color.name(QColor::HexRgb); // Default to hex
}

QString MainWindow::applyLightModeColor(const QString &originalColor) {
    QString cleanColor = originalColor.trimmed().toLower();
    if (colorMappings.contains(cleanColor)) {
        return colorMappings[cleanColor];
    }
    // Generic dark to light heuristic (can be improved)
    QColor color = stringToColor(cleanColor);
    if (color.isValid()) {
        int average = (color.red() + color.green() + color.blue()) / 3;
        if (average < 128 && color.alpha() > 128) { // If it's a dark, opaque color
            // Invert lightness, keep hue and saturation (simplified)
            return QString("rgb(%1,%2,%3)").arg(255-color.red()).arg(255-color.green()).arg(255-color.blue());
        }
    }
    return originalColor; // No change if no mapping or heuristic applies
}

QString MainWindow::processStyleSheetForLightMode(const QString &styleSheet) {
    QString result = styleSheet;
    QStringList colorProperties = {
        "background", "background-color", "color", "border", "border-color",
        "gridline-color", "selection-background-color", "selection-color"
    };

    for (const QString &prop : colorProperties) {
        QRegularExpression colorRegex(prop + "\\s*:\\s*([^;!]+)"); // Avoid matching !important
        QRegularExpressionMatchIterator matches = colorRegex.globalMatch(result);
        QString newResult = result;
        int offset = 0; // To adjust indices after replacements

        while (matches.hasNext()) {
            QRegularExpressionMatch match = matches.next();
            QString colorStr = match.captured(1).trimmed();

            // Skip if it's not a color or a complex value
            if (colorStr == "transparent" || colorStr == "none" || colorStr.contains("url(") || colorStr.contains("gradient(")) {
                continue;
            }
            QString newColorStr = applyLightModeColor(colorStr);
            if (newColorStr != colorStr) { // Only replace if a change occurred
                newResult.replace(match.capturedStart(1) + offset, match.capturedLength(1), newColorStr);
                offset += newColorStr.length() - match.capturedLength(1);
            }
        }
        result = newResult;
    }
    return result;
}

void MainWindow::applyLightModeToTable(QTableWidget* table) {
    if (!table) return;
    // Store original settings if not already done (simplified here)
    // if (!originalTableSettings.contains(table)) { /* ... */ }

    table->horizontalHeader()->setStyleSheet(
        "QHeaderView::section {"
        "    background-color: #2563eb;" // Blue for light mode header
        "    color: white;"
        "    padding: 6px; font-weight: bold; border: none;"
        "    border-right: 1px solid #1e40af; border-bottom: 2px solid #1e40af;"
        "}"
        );
    table->verticalHeader()->setStyleSheet(
        "QHeaderView::section {"
        "    background-color: #e2e8f0;" // Light gray for vertical header
        "    color: #334155;"             // Dark text
        "    padding: 4px; border: none;"
        "    border-bottom: 1px solid #cbd5e1; border-right: 2px solid #cbd5e1;"
        "}"
        );
    table->setGridStyle(Qt::SolidLine);
    table->setAlternatingRowColors(true);

    QString tableStyle = "QTableWidget {"
                         "    gridline-color: #e2e8f0;"
                         "    background-color: white;"
                         "    selection-background-color: #bfdbfe;" // Light blue selection
                         "    selection-color: #1e3a8a;"           // Dark blue text for selection
                         "    border: 1px solid #cbd5e1; border-radius: 4px;"
                         "}"
                         "QTableWidget::item { padding: 4px; border-bottom: 1px solid #f1f5f9; color: #334155; }" // Dark text for items
                         "QTableWidget::item:selected { background-color: #bfdbfe; color: #1e3a8a; }"
                         "QTableWidget::item:alternate { background-color: #f8fafc; }"; // Very light alternate rows
    table->setStyleSheet(tableStyle);
}

void MainWindow::applyLightModeToAllWidgets() {
    initializeColorMappings(); // Ensure mappings are up-to-date
    originalStylesheets.clear(); // Clear previous stored stylesheets

    QList<QWidget*> allWidgets = this->findChildren<QWidget*>();
    for (QWidget* widget : allWidgets) {
        originalStylesheets[widget] = widget->styleSheet(); // Store original
        widget->setStyleSheet(processStyleSheetForLightMode(widget->styleSheet()));
        if (QTableWidget* table = qobject_cast<QTableWidget*>(widget)) {
            applyLightModeToTable(table);
        }
        // Add specific styling for other widget types if needed
    }
    originalMainStylesheet = this->styleSheet(); // Store main window's original stylesheet
    this->setStyleSheet(processStyleSheetForLightMode(this->styleSheet()));

    // Light mode scrollbar style
    QString scrollbarStyle =
        "QScrollBar:vertical { background: #f1f5f9; width: 10px; margin: 0px; border-radius: 5px; }"
        "QScrollBar::handle:vertical { background: #94a3b8; min-height: 20px; border-radius: 5px; }"
        "QScrollBar::handle:vertical:hover { background: #64748b; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none; }"
        "QScrollBar:horizontal { background: #f1f5f9; height: 10px; margin: 0px; border-radius: 5px; }"
        "QScrollBar::handle:horizontal { background: #94a3b8; min-width: 20px; border-radius: 5px; }"
        "QScrollBar::handle:horizontal:hover { background: #64748b; }"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0px; }"
        "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal { background: none; }";
    if(qApp) qApp->setStyleSheet(qApp->styleSheet() + scrollbarStyle); // Append to global stylesheet
}

void MainWindow::restoreDarkModeToAllWidgets() {
    // Restore stylesheets from stored originals
    for (auto it = originalStylesheets.begin(); it != originalStylesheets.end(); ++it) {
        if (it.key()) { // Check if widget pointer is valid
            it.key()->setStyleSheet(it.value());
        }
    }
    this->setStyleSheet(originalMainStylesheet); // Restore main window stylesheet

    // Restore table styles (simplified - ideally revert to original QPalette or detailed settings)
    for (auto it = originalTableSettings.begin(); it != originalTableSettings.end(); ++it) {
        QTableWidget* table = it.key();
        if (!table) continue;
        // This is a simplified restoration; ideally, you'd restore all QPalette aspects
        // or revert to the stylesheet stored in originalStylesheets[table]
        if(originalStylesheets.contains(table)) {
            table->setStyleSheet(originalStylesheets[table]);
        } else {
            // Fallback if stylesheet wasn't stored for some reason
            table->setStyleSheet(""); // Clear specific light mode styles
        }
        // Re-apply dark theme header style if needed
        table->horizontalHeader()->setStyleSheet(
            "QHeaderView::section { background-color: #3a3a3a; color: white; /* ... other dark styles ... */ }"
            );
    }
    if(qApp) qApp->setStyleSheet("QLabel { color: #FFFFFF; }"); // Global dark mode label color
}

void MainWindow::enableLightMode() {
    if (isDarkMode) {
        isDarkMode = false; // Set state first
        QPropertyAnimation* opacityAnim = new QPropertyAnimation(this, "windowOpacity");
        opacityAnim->setDuration(150);
        opacityAnim->setStartValue(1.0);
        opacityAnim->setEndValue(0.9); // Slight fade out
        opacityAnim->setEasingCurve(QEasingCurve::InOutQuad);
        connect(opacityAnim, &QPropertyAnimation::finished, this, [=]() {
            applyLightModeToAllWidgets();
            if(qApp) qApp->setStyleSheet("QLabel { color: #000000; }"); // Black text for light mode
            if (m_salesChart) m_salesChart->setTheme(QChart::ChartThemeLight);
            if (m_salesSeries) m_salesSeries->setPen(QPen(Qt::blue, 2)); // Blue pen for light theme chart

            QPropertyAnimation* endAnim = new QPropertyAnimation(this, "windowOpacity");
            endAnim->setDuration(150);
            endAnim->setStartValue(0.9);
            endAnim->setEndValue(1.0); // Fade back in
            endAnim->setEasingCurve(QEasingCurve::InOutQuad);
            endAnim->start(QAbstractAnimation::DeleteWhenStopped);
        });
        opacityAnim->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void MainWindow::enableDarkMode() {
    if (!isDarkMode) {
        isDarkMode = true; // Set state first
        QPropertyAnimation* opacityAnim = new QPropertyAnimation(this, "windowOpacity");
        opacityAnim->setDuration(150);
        opacityAnim->setStartValue(1.0);
        opacityAnim->setEndValue(0.9);
        opacityAnim->setEasingCurve(QEasingCurve::InOutQuad);
        connect(opacityAnim, &QPropertyAnimation::finished, this, [=]() {
            restoreDarkModeToAllWidgets();
            if(qApp) qApp->setStyleSheet("QLabel { color: #FFFFFF; }"); // White text for dark mode
            if (m_salesChart) m_salesChart->setTheme(QChart::ChartThemeDark);
            if (m_salesSeries) m_salesSeries->setPen(QPen(Qt::cyan, 2)); // Cyan pen for dark theme chart

            QPropertyAnimation* endAnim = new QPropertyAnimation(this, "windowOpacity");
            endAnim->setDuration(150);
            endAnim->setStartValue(0.9);
            endAnim->setEndValue(1.0);
            endAnim->setEasingCurve(QEasingCurve::InOutQuad);
            endAnim->start(QAbstractAnimation::DeleteWhenStopped);
        });
        opacityAnim->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void MainWindow::setupChart() {
    m_salesSeries = new QLineSeries(this); // Parent `this` for auto-cleanup
    m_salesSeries->setName("Daily Sales Volume");

    m_salesChart = new QChart(); // No parent, ChartView takes ownership
    m_salesChart->addSeries(m_salesSeries);
    m_salesChart->setTitle("Sales Over Time");
    m_salesChart->setAnimationOptions(QChart::SeriesAnimations);

    QPen seriesPen((isDarkMode ? Qt::cyan : Qt::blue), 2);
    m_salesSeries->setPen(seriesPen);

    QDateTimeAxis *axisX = new QDateTimeAxis(this);
    axisX->setTickCount(10);
    axisX->setFormat("MMM dd");
    axisX->setTitleText("Date");
    m_salesChart->addAxis(axisX, Qt::AlignBottom);
    m_salesSeries->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis(this);
    axisY->setLabelFormat("%.2f Rs");
    axisY->setTitleText("Total Sales Amount");
    axisY->setMin(0);
    m_salesChart->addAxis(axisY, Qt::AlignLeft);
    m_salesSeries->attachAxis(axisY);

    m_salesChart->legend()->setVisible(true);
    m_salesChart->legend()->setAlignment(Qt::AlignBottom);

    if(isDarkMode) {
        m_salesChart->setTheme(QChart::ChartThemeDark);
    } else {
        m_salesChart->setTheme(QChart::ChartThemeLight);
    }

    m_chartView = new QChartView(m_salesChart); // ChartView takes ownership of m_salesChart
    m_chartView->setRenderHint(QPainter::Antialiasing);

    if (ui->horizontalFrame) {
        QLayout *oldLayout = ui->horizontalFrame->layout();
        if (oldLayout) {
            QLayoutItem* item;
            while ((item = oldLayout->takeAt(0)) != nullptr) {
                if (item->widget()) {
                    item->widget()->deleteLater();
                }
                delete item;
            }
            delete oldLayout;
        }
        QVBoxLayout *layout = new QVBoxLayout();
        layout->addWidget(m_chartView);
        ui->horizontalFrame->setLayout(layout);
    } else {
        qDebug() << "ui->horizontalFrame is null. Cannot add sales chart.";
    }
    if (ui->horizontalFrame) {
        QLayout *oldLayout = ui->horizontalFrame_2->layout();
        if (oldLayout) {
            QLayoutItem* item;
            while ((item = oldLayout->takeAt(0)) != nullptr) {
                if (item->widget()) {
                    item->widget()->deleteLater();
                }
                delete item;
            }
            delete oldLayout;
        }
        QVBoxLayout *layout = new QVBoxLayout();
        layout->addWidget(m_chartView);
        ui->horizontalFrame_2->setLayout(layout);
    } else {
        qDebug() << "ui->horizontalFrame is null. Cannot add sales chart.";
    }
}

void MainWindow::updateSalesChart() {
    if (!m_salesSeries || !m_dbHandler || !m_dbHandler->isConnected() || !m_salesChart) {
        qDebug() << "Sales chart update prerequisites not met.";
        return;
    }

    m_salesSeries->clear();
    QSqlQuery query(m_dbHandler->getDatabase());
    query.prepare("SELECT DATE(sale_date), SUM(total_price) FROM Sales GROUP BY DATE(sale_date) ORDER BY DATE(sale_date) ASC LIMIT 30");

    double maxSalesValue = 0.0;
    QDateTime minDataDate;
    QDateTime maxDataDate;
    bool dataFound = false;

    if (query.exec()) {
        while (query.next()) {
            dataFound = true;
            QDate dateOnly = query.value(0).toDate();
            QDateTime dateTime = dateOnly.startOfDay(); // Use QDateTime for the axis
            double totalSales = query.value(1).toDouble();

            m_salesSeries->append(dateTime.toMSecsSinceEpoch(), totalSales);

            if (totalSales > maxSalesValue) maxSalesValue = totalSales;
            if (minDataDate.isNull() || dateTime < minDataDate) minDataDate = dateTime;
            if (maxDataDate.isNull() || dateTime > maxDataDate) maxDataDate = dateTime;
        }
    } else {
        qDebug() << "Failed to fetch sales data for chart:" << query.lastError().text();
    }

    QList<QAbstractAxis*> axesXList = m_salesChart->axes(Qt::Horizontal, m_salesSeries);
    QList<QAbstractAxis*> axesYList = m_salesChart->axes(Qt::Vertical, m_salesSeries);

    QDateTimeAxis *axisX = axesXList.isEmpty() ? nullptr : qobject_cast<QDateTimeAxis*>(axesXList.first());
    QValueAxis *axisY = axesYList.isEmpty() ? nullptr : qobject_cast<QValueAxis*>(axesYList.first());

    if (!axisX || !axisY) {
        qDebug() << "Chart axes not found or of wrong type. Recreating or skipping update.";
        return; // Or attempt to re-attach/re-create axes
    }

    if (!dataFound) {
        minDataDate = QDateTime::currentDateTime().addDays(-7);
        maxDataDate = QDateTime::currentDateTime();
        axisY->setRange(0, 1000); // Default if no data
    } else {
        if (minDataDate == maxDataDate) { // Handle single data point
            minDataDate = minDataDate.addDays(-1);
            maxDataDate = maxDataDate.addDays(1);
        }
        axisY->setRange(0, maxSalesValue > 0 ? (maxSalesValue * 1.1) : 100.0); // Dynamic range with padding
    }
    axisX->setRange(minDataDate, maxDataDate);
}


void MainWindow::setupCalculator() {
    if (!ui->gridLayout_2 || !ui->display_2) return;

    ui->gridLayout_2->addWidget(ui->display_2, 0, 0, 1, 3);
    QPushButton *backspaceBtn = new QPushButton("⌫", this); // Parented
    backspaceBtn->setFixedSize(60, 60);
    ui->gridLayout_2->addWidget(backspaceBtn, 0, 3);
    connect(backspaceBtn, &QPushButton::clicked, this, [this]() {
        if(ui->display_2) {
            QString text = ui->display_2->text();
            if (!text.isEmpty()) ui->display_2->setText(text.left(text.length() - 1));
        }
    });

    QStringList buttons = {"7", "8", "9", "C", "4", "5", "6", "*", "1", "2", "3", "-", "0", ".", "=", "+"};
    int pos = 0;
    for (int row = 1; row <= 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            if (pos >= buttons.size()) break;
            QString label = buttons[pos++];
            QPushButton *btn = new QPushButton(label, this); // Parented
            btn->setFixedSize(60, 60);
            ui->gridLayout_2->addWidget(btn, row, col);
            connect(btn, &QPushButton::clicked, this, [this, label]() {
                if(!ui->display_2) return;
                if (label == "=") {
                    QJSEngine engine;
                    QString expression = ui->display_2->text();
                    // Basic validation to prevent harmful evaluation, can be improved
                    if (expression.contains(QRegularExpression("[^0-9().*/+\\-%\\s]"))) {
                        ui->display_2->setText("Invalid Char"); return;
                    }
                    QJSValue result = engine.evaluate(expression);
                    if (result.isError()) {
                        ui->display_2->setText("Error");
                    } else {
                        ui->display_2->setText(result.toString());
                    }
                } else if (label == "C") {
                    ui->display_2->clear();
                } else {
                    ui->display_2->setText(ui->display_2->text() + label);
                }
            });
        }
    }
}

void MainWindow::connectPageButton(QPushButton *button, int index) {
    if (button && ui->stackedWidget && index >= 0 && index < ui->stackedWidget->count()) {
        connect(button, &QPushButton::clicked, this, [this, index]() {
            ui->stackedWidget->setCurrentIndex(index);
        });
    } else if (button) {
        qDebug() << "Failed to connect navigation for button" << button->objectName() << "to index" << index << ". StackedWidget count:" << (ui->stackedWidget ? ui->stackedWidget->count() : -1);
    }
}

void MainWindow::setupNavigation() {
    int salesDashboardActualIndex = -1;
    if (m_salesdashboard && ui->stackedWidget) {
        salesDashboardActualIndex = ui->stackedWidget->indexOf(m_salesdashboard);
        if(salesDashboardActualIndex == -1) {
            qDebug() << "WARNING: SalesDashboard instance exists but not found in stackedWidget!";
        }
    }

    QMap<QPushButton*, int> navMap = {
        {ui->page1, 1}, {ui->page2, 2}, {ui->page3, 3}, {ui->page4, 4}, {ui->page5, 5},
        {ui->page6, salesDashboardActualIndex != -1 ? salesDashboardActualIndex : 6}, // Page 6 -> Sales Dashboard
        {ui->page7, 7},
        {ui->page21, 1}, {ui->page22, 2}, {ui->page23, 3}, {ui->page24, 4}, {ui->page25, 5},
        {ui->page26, salesDashboardActualIndex != -1 ? salesDashboardActualIndex : 6},
        {ui->page27, 7},
        // ... (repeat for page3X, page4X, etc. ensuring correct Sales Dashboard index)
        {ui->page31, 1}, {ui->page32, 2}, {ui->page33, 3}, {ui->page34, 4}, {ui->page35, 5},
        {ui->page36, salesDashboardActualIndex != -1 ? salesDashboardActualIndex : 6},
        {ui->page37, 7},
        {ui->page41, 1}, {ui->page42, 2}, {ui->page43, 3}, {ui->page44, 4}, {ui->page45, 5},
        {ui->page46, salesDashboardActualIndex != -1 ? salesDashboardActualIndex : 6},
        {ui->page47, 7},
        {ui->page51, 1}, {ui->page52, 2}, {ui->page53, 3}, {ui->page54, 4}, {ui->page55, 5},
        {ui->page56, salesDashboardActualIndex != -1 ? salesDashboardActualIndex : 6},
        {ui->page57, 7},
        {ui->page61, 1}, {ui->page62, 2}, {ui->page63, 3}, {ui->page64, 4}, {ui->page65, 5},
        {ui->page66, salesDashboardActualIndex != -1 ? salesDashboardActualIndex : 6},
        {ui->page67, 7},
        {ui->page71, 1}, {ui->page72, 2}, {ui->page73, 3}, {ui->page74, 4}, {ui->page75, 5},
        {ui->page76, salesDashboardActualIndex != -1 ? salesDashboardActualIndex : 6},
        {ui->page77, 7},
        // Worker pages
        {ui->pagew1, 8}, {ui->pagew2, 9}, {ui->pagew3, 10}, {ui->pagew4, 11},
        {ui->pagew21, 8}, {ui->pagew22, 9}, {ui->pagew23, 10}, {ui->pagew24, 11},
        {ui->pagew31, 8}, {ui->pagew32, 9}, {ui->pagew33, 10}, {ui->pagew34, 11},
        // Worker Sales (assuming these point to worker-specific views or the main sales dashboard if applicable)
        {ui->pages1_2, 8}, {ui->pages2_2, 9}, {ui->pages3_2, 10}, {ui->pages4_2, 11},
        // Action buttons
        {ui->cross, 7}, // Example: cross button on a page goes to workers main page
        {ui->addProductBtn, 14}, {ui->addWorkerBtn, 12}, {ui->addDebtorBtn, 13},
        {ui->cross_3, 2}, // Example: cross button on add debtor page goes back to debtor list
        {ui->addProductBtn_4, 14}, // addProductBtn_4 also goes to add product page
        {ui->addVendorBtn, 15}
        // {ui->addVendorBtn_2, 5} // This was in your repo, ensure '5' is correct target for submitting vendor
        // {ui->addWorkerBtn_2, 7} // Ensure '7' is correct target for submitting worker
        // {ui->addDebtorBtn_2, 2} // Ensure '2' is correct target for submitting debtor
    };

    for (auto it = navMap.begin(); it != navMap.end(); ++it) {
        if (it.key() && it.value() != -1) {
            connectPageButton(it.key(), it.value());
        } else if (it.key()) { // If value is -1, it means sales dashboard wasn't found for a button that needs it
            qDebug() << "Navigation for" << it.key()->objectName() << "to SalesDashboard failed (index was -1).";
        }
    }

    // Logout Buttons
    QList<QPushButton*> logoutButtons = {
        ui->btnlogout_8, ui->btnlogout_2, ui->btnlogout_3, ui->btnlogout_4, ui->btnlogout_5,
        ui->btnlogout_6, ui->btnlogout_7, ui->btnlogout_9, ui->btnlogout_10, ui->btnlogout_11,
        ui->btnlogout_12
    };
    for(QPushButton* btn : logoutButtons){
        if(btn) connect(btn, &QPushButton::clicked, this, &MainWindow::logoutUser);
    }
}

void MainWindow::on_loginbtn_clicked() {
    if (!m_dbHandler || !ui->username_login || !ui->password_login || !ui->stackedWidget) return;
    QString username = ui->username_login->text();
    QString password = ui->password_login->text();

    if (m_dbHandler->login(username, password)) {
        ui->stackedWidget->setCurrentIndex(m_dbHandler->isAdmin() ? 1 : 8); // Admin or Worker dashboard
        updateDashboard(); // Update dashboard after login
    } else {
        QMessageBox::warning(this, "Login Failed", "Invalid username or password. Please try again.");
    }
}

void MainWindow::logoutUser() {
    if (m_dbHandler) m_dbHandler->logout();
    if(ui->username_login) ui->username_login->clear();
    if(ui->password_login) ui->password_login->clear();
    if(ui->stackedWidget) ui->stackedWidget->setCurrentIndex(0); // Go to login page
}

void MainWindow::on_eyeButton_clicked() {
    if(!ui->password_login || !ui->eyeButton) return;
    passwordVisible = !passwordVisible;
    ui->password_login->setEchoMode(passwordVisible ? QLineEdit::Normal : QLineEdit::Password);
    QString iconPath = passwordVisible ?
                           (isDarkMode ? ":/new/prefix1/images/eyebrow (1).png" : ":/new/prefix1/images/eyebrow.png") :
                           (isDarkMode ? ":/new/prefix1/images/view-Stroke-Rounded.png" : ":/new/prefix1/images/view-Stroke-Rounded (1).png");
    ui->eyeButton->setIcon(QIcon(iconPath));
}

// --- Manager Setup Functions ---
void MainWindow::setupDebtManager() {
    m_debtManager = new DebtManager(m_dbHandler, this);
    connect(m_debtManager, &DebtManager::debtorsUpdated, this, &MainWindow::onDebtorsUpdated);
    if(ui->debtorsTable) setupTableWidget(ui->debtorsTable, {"ID", "Name", "Contact", "Address", "Amount", "Date"});
    refreshDebtorTable();
}
void MainWindow::setupProductManager() {
    m_productManager = new ProductManager(m_dbHandler, this);
    connect(m_productManager, &ProductManager::productsUpdated, this, &MainWindow::onProductsUpdated);
    if(ui->productsTable) setupTableWidget(ui->productsTable, {"ID", "Name", "Price", "Category", "Quantity", "Added At"});
    if(ui->productsTable_2) setupTableWidget(ui->productsTable_2, {"ID", "Name", "Price", "Category", "Quantity", "Added At"}); // For worker product view
    refreshProductTable();
    if(ui->categoryCombo) ui->categoryCombo->addItems({"Electronics", "Clothing", "Food", "Beverages", "Household", "Other"});
}
void MainWindow::setupVendorManager() {
    m_vendorManager = new VendorManager(m_dbHandler, this);
    connect(m_vendorManager, &VendorManager::vendorsUpdated, this, &MainWindow::onVendorsUpdated);
    if(ui->vendorsTable) setupTableHeaders(ui->vendorsTable, {"ID", "Name", "Contact", "Address", "Payment", "Date of Supply"});
    refreshVendorTable();
}
void MainWindow::setupWorkerManager() {
    m_workManager = new WorkerManager(m_dbHandler, this);
    connect(m_workManager, &WorkerManager::workersUpdated, this, &MainWindow::onWorkersUpdated);
    if(ui->workersTable) setupTableHeaders(ui->workersTable, {"ID", "Name", "Contact", "Email", "Status", "Salary", "Date of Joining"});
    refreshWorkerTable();
    if(ui->statusCombo) {
        ui->statusCombo->clear(); // Ensure it's empty before adding
        ui->statusCombo->addItems({"Active", "On Leave", "Terminated", "Suspended", "Part-time"});
    }
}
void MainWindow::setupStockManager() {
    m_stockManager = new StockManager(m_dbHandler, this);
    connect(m_stockManager, &StockManager::stockUpdated, this, &MainWindow::onStockUpdated);
    QStringList headers = {"Product ID", "Product Name", "Price/Unit", "Category","Total Quantity", "Remaining Quantity"};
    if(ui->stockTable) setupTableHeaders(ui->stockTable, headers);
    if(ui->workerStockTable) setupTableHeaders(ui->workerStockTable, headers);
    refreshStockTable();
}

// --- Table Setup ---
void MainWindow::setupTableWidget(QTableWidget *table, const QStringList &headers) {
    if (!table) { qDebug() << "setupTableWidget: table is null"; return; }
    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->verticalHeader()->setVisible(false);
    table->setAlternatingRowColors(true);
}
void MainWindow::setupTableHeaders(QTableWidget *table, const QStringList &headers) {
    // This is identical to setupTableWidget; can be consolidated if desired.
    setupTableWidget(table, headers);
}

// --- Refresh Functions ---
void MainWindow::refreshDebtorTable() { if (m_debtManager && m_dbHandler && m_dbHandler->isConnected() && ui->debtorsTable) m_debtManager->loadDebtors(ui->debtorsTable); }
void MainWindow::refreshProductTable() {
    if (m_productManager && m_dbHandler && m_dbHandler->isConnected()) {
        if(ui->productsTable) m_productManager->loadProducts(ui->productsTable);
        if(ui->productsTable_2) m_productManager->loadProducts(ui->productsTable_2);
    }
}
void MainWindow::refreshVendorTable() { if (m_vendorManager && m_dbHandler && m_dbHandler->isConnected() && ui->vendorsTable) m_vendorManager->loadVendors(ui->vendorsTable); }
void MainWindow::refreshWorkerTable() { if (m_workManager && m_dbHandler && m_dbHandler->isConnected() && ui->workersTable) m_workManager->loadWorkers(ui->workersTable); }
void MainWindow::refreshStockTable() {
    if (m_stockManager && m_dbHandler && m_dbHandler->isConnected()) {
        if(ui->stockTable) m_stockManager->loadStock(ui->stockTable);
        if(ui->workerStockTable) m_stockManager->loadStock(ui->workerStockTable);
    }
}

// --- Update Handlers (Slots) ---
void MainWindow::onDebtorsUpdated() { refreshDebtorTable(); updateDashboard(); }
void MainWindow::onProductsUpdated() { refreshProductTable(); updateDashboard(); refreshProductSalesTable(); /* Sales dashboard might need this */ }
void MainWindow::onVendorsUpdated() { refreshVendorTable(); updateDashboard(); }
void MainWindow::onWorkersUpdated() { refreshWorkerTable(); updateDashboard(); }
void MainWindow::onStockUpdated() { refreshStockTable(); updateDashboard(); refreshProductSalesTable(); /* Sales depends on product stock */}
void MainWindow::onSalesUpdated() {
    refreshSalesTable();            // Sales history table
    refreshProductSalesTable();     // Product list for new sales (stock updated)
    refreshStockTable();            // General stock views
    updateDashboard();              // Main dashboard stats and chart
    if (m_salesdashboard) m_salesdashboard->refreshData(); // The dedicated SalesDashboard widget
}

void MainWindow::updateDashboard() {
    if (!m_dbHandler || !m_dbHandler->isConnected()) return;
    int totalDebtors = 0; double totalDebt = 0.0;
    if (m_debtManager && m_debtManager->getDebtorStats(totalDebtors, totalDebt)) {
        if(ui->labelTotalDebtors) ui->labelTotalDebtors->setText(QString::number(totalDebtors));
        if(ui->workerTotalDebtorsLabel) ui->workerTotalDebtorsLabel->setText(QString::number(totalDebtors));
        if(ui->workerLabelTotalDebt) ui->workerLabelTotalDebt->setText(QString("%1 Rs.").arg(totalDebt, 0, 'f', 2));
    }
    int totalProducts = 0; int totalStock = 0;
    if (m_productManager && m_productManager->getProductStats(totalProducts, totalStock)) {
        if(ui->labelTotalProducts) ui->labelTotalProducts->setText(QString::number(totalProducts));
        if(ui->workerTotalProductsLabel) ui->workerTotalProductsLabel->setText(QString::number(totalProducts));
        // Update labelTotalStock if you have one
    }
    int numSales = 0; double totalSalesAmount = 0.0; double profit = 0.0; // Assuming profit is calculated by SalesManager
    if (m_salesManager && m_salesManager->getSalesStats(numSales, totalSalesAmount, profit)) {
        if(ui->labelTotalSales) ui->labelTotalSales->setText(QString::number(numSales));
        if(ui->labelTotalAmount) ui->labelTotalAmount->setText(QString("%1 Rs.").arg(totalSalesAmount, 0, 'f', 2));
        if(ui->workerTotalAmountLabel) ui->workerTotalAmountLabel->setText(QString("%1 Rs.").arg(totalSalesAmount, 0, 'f', 2));
        if(ui->workerTotalSalesLabel) ui->workerTotalSalesLabel->setText(QString::number(numSales));
        // Update profit label if you have one
    }
    updateSalesChart(); // Refresh the chart with new data
}

// --- Form/Input Helpers ---
void MainWindow::clearForm(const QList<QLineEdit*> &fields) { for (auto field : fields) if(field) field->clear(); }
bool MainWindow::validateInput(const QStringList &fields) { return std::all_of(fields.begin(), fields.end(), [](const QString &f){ return !f.isEmpty(); }); }
// std::optional<int> MainWindow::getSelectedId(QTableWidget *table, const QString &type) {
//     if (!table || table->selectedItems().isEmpty() || table->currentRow() < 0) {
//         showWarning(QString("Please select a %1 to remove.").arg(type));
//         return std::nullopt;
//     }
//     bool ok;
//     int id = table->item(table->currentRow(), 0)->text().toInt(&ok); // Use currentRow() for reliability
//     if (!ok) {
//         showError(QString("Invalid %1 ID selected.").arg(type));
//         return std::nullopt;
//     }
//     return id;
// }

// --- Message Boxes ---
void MainWindow::showDarkMessageBox(const QString &title, const QString &message) {
    QMessageBox msgBox(this); msgBox.setWindowTitle(title); msgBox.setText(message);
    msgBox.setIcon(QMessageBox::Warning); // Or Information, Critical based on context
    // Style it based on current theme (isDarkMode)
    if (isDarkMode) {
        msgBox.setStyleSheet(
            "QMessageBox { background-color: #2b2b2b; color: white; border: 1px solid #444; }"
            "QMessageBox QLabel { color: white; } "
            "QPushButton { background-color: #444; color: white; padding: 5px 10px; border-radius: 3px; min-width: 70px; border: 1px solid #555; }"
            "QPushButton:hover { background-color: #555; }"
            "QPushButton:pressed { background-color: #333; }"
            );
    } else {
        msgBox.setStyleSheet( // Basic light theme style
            "QMessageBox { background-color: #f0f0f0; color: black; }"
            "QMessageBox QLabel { color: black; }"
            "QPushButton { background-color: #e1e1e1; color: black; padding: 5px 10px; border-radius: 3px; min-width: 70px; border: 1px solid #c0c0c0;}"
            "QPushButton:hover { background-color: #d1d1d1; }"
            );
    }
    msgBox.exec();
}
bool MainWindow::confirmRemoval(const QString &type, const QString &name) {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirm Removal", QString("Are you sure you want to remove %1: %2?").arg(type, name), QMessageBox::Yes | QMessageBox::No);
    return reply == QMessageBox::Yes;
}
void MainWindow::showSuccess(const QString &message) { QMessageBox::information(this, "Success", message); }
void MainWindow::showError(const QString &message) { QMessageBox::critical(this, "Error", message); }
void MainWindow::showWarning(const QString &message) { QMessageBox::warning(this, "Warning", message); }
bool MainWindow::showSuccessWithOk(const QString &message) {
    QMessageBox msgBox(this); msgBox.setText(message); msgBox.setWindowTitle("Success");
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setStandardButtons(QMessageBox::Ok); return msgBox.exec() == QMessageBox::Ok;
}

// --- Debtor Management Slots & Helpers ---
std::tuple<QString, QString, QString, double, QDate> MainWindow::getDebtorFormData() {
    return {ui->debtorNameEdit ? ui->debtorNameEdit->text().trimmed() : "",
            ui->debtorContactEdit ? ui->debtorContactEdit->text().trimmed() : "",
            ui->debtorAddressEdit ? ui->debtorAddressEdit->text().trimmed() : "", // Assuming email is part of address or not used
            ui->debtorAmountEdit ? ui->debtorAmountEdit->text().toDouble() : 0.0,
            ui->dateEdit ? ui->dateEdit->date() : QDate::currentDate()};
}
bool MainWindow::validateDebtorInput(const QString &name, const QString &contact, const QString &address, double amount) {
    if (name.isEmpty() || contact.isEmpty() || address.isEmpty() || amount <= 0) {
        showWarning("Please fill all fields for the debtor and ensure the amount is positive."); return false;
    } return true;
}
void MainWindow::clearDebtorForm() {
    clearForm({ui->debtorNameEdit, ui->debtorContactEdit, ui->debtorAddressEdit, ui->debtorAmountEdit});
    if(ui->dateEdit) ui->dateEdit->setDate(QDate::currentDate());
}
void MainWindow::on_addDebtorBtn_clicked() { clearDebtorForm(); if(ui->stackedWidget) ui->stackedWidget->setCurrentIndex(13); } // Page for adding debtor
void MainWindow::on_addDebtorBtn_2_clicked() { // Submit debtor form
    if(!m_debtManager) return;
    auto [name, contact, address, amount, date] = getDebtorFormData();
    if (!validateDebtorInput(name, contact, address, amount)) return;
    // The addDebtor in DebtManager takes name, contact, email, address, amount, date.
    // Assuming email is optional or included in address for this form. If email is separate, adjust getDebtorFormData.
    if (m_debtManager->addDebtor(name, contact, address, amount, date)) { // Passing empty string for email
        showSuccess("Debtor added successfully.");
        if(ui->stackedWidget) ui->stackedWidget->setCurrentIndex(2); // Debtor list page
    } else { showError("Failed to add debtor. Please try again."); }
}
void MainWindow::on_removeDebtorBtn_clicked() {
    if(!m_debtManager || !ui->debtorsTable) return;
    auto idOpt = getSelectedId(ui->debtorsTable, "debtor");
    if (!idOpt) return;
    QString name = ui->debtorsTable->item(ui->debtorsTable->currentRow(), 1)->text(); // Assuming name is in column 1
    if (confirmRemoval("debtor", name)) {
        if (m_debtManager->removeDebtor(*idOpt)) {
            showSuccess("Debtor removed successfully.");
        } else {
            showError("Failed to remove debtor.");
        }
    }
}
void MainWindow::on_debtorSearchEdit_textChanged(const QString &searchText) {
    if (m_debtManager && m_dbHandler && m_dbHandler->isConnected() && ui->debtorsTable) {
        if (searchText.isEmpty()) refreshDebtorTable();
        else m_debtManager->searchDebtors(ui->debtorsTable, searchText);
    }
}

// --- Product Management Slots & Helpers ---
std::tuple<QString, double, QString, int, QDate> MainWindow::getProductFormData() {
    return {ui->productNameEdit ? ui->productNameEdit->text().trimmed() : "",
            ui->priceEdit ? ui->priceEdit->text().toDouble() : 0.0,
            ui->categoryCombo ? ui->categoryCombo->currentText() : "",
            ui->quantityEdit ? ui->quantityEdit->text().toInt() : 0,
            ui->dateEdit_2 ? ui->dateEdit_2->date() : QDate::currentDate()};
}
bool MainWindow::validateProductInput(const QString &name, double price, int quantity) {
    if (name.isEmpty() || price <= 0 || quantity < 0) {
        showWarning("Product name, valid price (>0), and non-negative quantity are required."); return false;
    } return true;
}
void MainWindow::clearProductForm() {
    clearForm({ui->productNameEdit, ui->priceEdit, ui->quantityEdit});
    if(ui->categoryCombo) ui->categoryCombo->setCurrentIndex(0);
    if(ui->dateEdit_2) ui->dateEdit_2->setDate(QDate::currentDate());
}
void MainWindow::on_addProductBtn_clicked() { clearProductForm(); if(ui->stackedWidget) ui->stackedWidget->setCurrentIndex(14); } // Add product page
void MainWindow::on_addProductBtn_4_clicked() { clearProductForm(); if(ui->stackedWidget) ui->stackedWidget->setCurrentIndex(14); } // Another button for add product page
void MainWindow::on_addProductBtn_2_clicked() { // Submit product form
    if(!m_productManager) return;
    auto [name, price, category, quantity, date] = getProductFormData();
    if (!validateProductInput(name, price, quantity)) return;
    if (m_productManager->addProduct(name, price, category, quantity, date)) {
        showSuccessWithOk("Product added successfully!"); // Returns to previous screen on OK
        clearProductForm();
        if(ui->stackedWidget) ui->stackedWidget->setCurrentIndex(3); // Product list page
    } else { showError("Failed to add product. Please try again."); }
}
void MainWindow::on_removeProductBtn_clicked() {
    if(!m_productManager || !ui->productsTable) return;
    auto idOpt = getSelectedId(ui->productsTable, "product");
    if (!idOpt) return;
    QString name = ui->productsTable->item(ui->productsTable->currentRow(), 1)->text();
    if (confirmRemoval("product", name)) {
        if (m_productManager->removeProduct(*idOpt)) {
            showSuccess("Product removed successfully.");
        } else {
            showError("Failed to remove product.");
        }
    }
}
void MainWindow::on_productSearchEdit_textChanged(const QString &searchText) { // Admin product search
    if (m_productManager && m_dbHandler && m_dbHandler->isConnected() && ui->productsTable) {
        if (searchText.isEmpty()) refreshProductTable(); // Refreshes both admin and worker tables
        else m_productManager->searchProducts(ui->productsTable, searchText);
    }
}
void MainWindow::on_workerProductSearchEdit_textChanged(const QString &searchText){ // Worker product search (usually on productsTable_2)
    if (m_productManager && m_dbHandler && m_dbHandler->isConnected() && ui->productsTable_2) {
        if(searchText.isEmpty()) m_productManager->loadProducts(ui->productsTable_2); // Or call refreshProductTable
        else m_productManager->searchProducts(ui->productsTable_2, searchText);
    }
}

// --- Vendor Management Slots & Helpers ---
void MainWindow::on_addVendorBtn_clicked() { // Open add vendor page
    clearForm({ui->vendorNameEdit, ui->vendorContactEdit, ui->vendorAddressEdit, ui->vendorPaymentEdit});
    if(ui->dateEdit_4) ui->dateEdit_4->setDate(QDate::currentDate());
    if(ui->stackedWidget) ui->stackedWidget->setCurrentIndex(15);
}
void MainWindow::on_addVendorBtn_2_clicked() { // Submit vendor form
    if(!m_vendorManager || !ui->vendorNameEdit || !ui->vendorContactEdit || !ui->vendorAddressEdit || !ui->vendorPaymentEdit || !ui->dateEdit_4) return;
    QString name = ui->vendorNameEdit->text().trimmed();
    QString contact = ui->vendorContactEdit->text().trimmed();
    QString address = ui->vendorAddressEdit->text().trimmed();
    QString paymentStr = ui->vendorPaymentEdit->text().trimmed();
    if (!validateInput({name, contact, address, paymentStr})) {
        showDarkMessageBox("Error", "All vendor fields are required."); return;
    }
    bool ok; double payment = paymentStr.toDouble(&ok);
    if(!ok || payment < 0){ showDarkMessageBox("Error", "Invalid payment amount for vendor."); return; }

    if (m_vendorManager->addVendor(name, address, contact, payment, ui->dateEdit_4->date())) {
        showDarkMessageBox("Success", "Vendor added successfully!");
        if(ui->stackedWidget) ui->stackedWidget->setCurrentIndex(4); // Vendor list page
    } else { showDarkMessageBox("Error", "Failed to add vendor."); }
}
void MainWindow::on_removeVendorBtn_clicked() {
    if(!m_vendorManager || !ui->vendorsTable) return;
    auto idOpt = getSelectedId(ui->vendorsTable, "vendor");
    if (!idOpt) return;
    QString name = ui->vendorsTable->item(ui->vendorsTable->currentRow(), 1)->text();
    if (confirmRemoval("vendor", name)) {
        if (m_vendorManager->removeVendor(*idOpt)) {
            showDarkMessageBox("Success", "Vendor removed successfully!");
        } else {
            showDarkMessageBox("Error", "Failed to remove vendor.");
        }
    }
}
void MainWindow::on_vendorSearchEdit_textChanged(const QString &searchText) {
    if (m_vendorManager && m_dbHandler && m_dbHandler->isConnected() && ui->vendorsTable) {
        if (searchText.isEmpty()) refreshVendorTable();
        else m_vendorManager->searchVendors(ui->vendorsTable, searchText);
    }
}

// --- Worker Management Slots & Helpers ---
void MainWindow::on_addWorkerBtn_clicked() { // Open add worker page
    clearForm({ui->workerNameEdit, ui->workerContactEdit, ui->workerEmailEdit, ui->workerSalaryEdit});
    if(ui->statusCombo) ui->statusCombo->setCurrentIndex(0);
    if(ui->dateEdit_3) ui->dateEdit_3->setDate(QDate::currentDate());
    if(ui->stackedWidget) ui->stackedWidget->setCurrentIndex(12);
}
void MainWindow::on_addWorkerBtn_2_clicked() { // Submit worker form
    if(!m_workManager || !ui->workerNameEdit || !ui->workerContactEdit || !ui->workerEmailEdit || !ui->workerSalaryEdit || !ui->statusCombo || !ui->dateEdit_3) return;
    QString name = ui->workerNameEdit->text().trimmed();
    QString contact = ui->workerContactEdit->text().trimmed();
    QString email = ui->workerEmailEdit->text().trimmed(); // Email can be optional depending on DB schema
    QString salaryStr = ui->workerSalaryEdit->text().trimmed();

    if (!validateInput({name, contact, salaryStr})) { // Email might be optional
        showDarkMessageBox("Error", "Worker Name, Contact, and Salary are required."); return;
    }
    bool ok; double salary = salaryStr.toDouble(&ok);
    if(!ok || salary < 0){ showDarkMessageBox("Error", "Invalid salary amount for worker."); return; }

    if (m_workManager->addWorker(name, contact, email, ui->statusCombo->currentText(), salary, ui->dateEdit_3->date())) {
        showDarkMessageBox("Success", "Worker added successfully!");
        if(ui->stackedWidget) ui->stackedWidget->setCurrentIndex(7); // Worker list page
    } else { showDarkMessageBox("Error", "Failed to add worker."); }
}
void MainWindow::on_removeWorkerBtn_clicked() {
    if(!m_workManager || !ui->workersTable) return;
    auto idOpt = getSelectedId(ui->workersTable, "worker");
    if (!idOpt) return;
    QString name = ui->workersTable->item(ui->workersTable->currentRow(), 1)->text();
    if (confirmRemoval("worker", name)) {
        if (m_workManager->removeWorker(*idOpt)) {
            showDarkMessageBox("Success", "Worker removed successfully!");
        } else {
            showDarkMessageBox("Error", "Failed to remove worker.");
        }
    }
}
void MainWindow::on_workerSearchEdit_textChanged(const QString &searchText) { // Admin worker search
    if (m_workManager && m_dbHandler && m_dbHandler->isConnected() && ui->workersTable) {
        if(searchText.isEmpty()) refreshWorkerTable();
        else m_workManager->searchWorkers(ui->workersTable, searchText);
    }
}

// --- Stock Management Slots ---
void MainWindow::on_viewStockSearchEdit_textChanged(const QString &searchText) { // Admin stock search
    if (m_stockManager && m_dbHandler && m_dbHandler->isConnected() && ui->stockTable) {
        if(searchText.isEmpty()) refreshStockTable(); // Refreshes both admin and worker stock tables
        else m_stockManager->searchStock(ui->stockTable, searchText);
    }
}
void MainWindow::on_workerStockSearchEdit_textChanged(const QString &searchText) { // Worker stock search
    if (m_stockManager && m_dbHandler && m_dbHandler->isConnected() && ui->workerStockTable) {
        if(searchText.isEmpty()) m_stockManager->loadStock(ui->workerStockTable); // Or call refreshStockTable
        else m_stockManager->searchStock(ui->workerStockTable, searchText);
    }
}

std::optional<int> MainWindow::getSelectedId(QTableWidget *table, const QString &type)
{
    if (!table || table->selectedItems().isEmpty()) {
        showDarkMessageBox("Error", QString("Please select a %1 to remove!").arg(type));
        return std::nullopt;
    }

    bool ok;
    int id = table->item(table->selectedItems().first()->row(), 0)->text().toInt(&ok);

    if (!ok) {
        showDarkMessageBox("Error", QString("Invalid %1 ID!").arg(type));
        return std::nullopt;
    }

    return id;
}
void MainWindow::setupSalesManager()
{
    m_salesManager = new SalesManager(m_dbHandler, this);

    // Initialize member variables
    m_selectedItems.clear();
    m_totalAmount = 0.0;
    m_currentSelectedRow = -1;

    // Connect signals
    connect(m_salesManager, &SalesManager::salesUpdated, this, &MainWindow::onSalesUpdated);

    // Setup tables with proper headers
    setupSalesTable(ui->searchProductTable, {"ID", "Name", "Price", "Category", "Stock", "Updated"}, 6);
    setupSalesTable(ui->selectedProductsTable, {"Product", "Price", "Qty", "Total", "Remove"}, 5);
    setupSalesTable(ui->salesTable, {"Sales ID", "Salesman ID", "Product ID", "Product Name", "Price", "Category", "Quantity Sold", "Date/Time"}, 8);

    // Connect UI signals


    // Initial data load
    refreshProductSalesTable();
    refreshSalesTable();
}

void MainWindow::setupSalesTable(QTableWidget *table, const QStringList &headers, int columnCount)
{
    if (!table) return;

    table->setColumnCount(columnCount);
    table->setHorizontalHeaderLabels(headers);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->verticalHeader()->setVisible(false);
    table->setAlternatingRowColors(true);
}

void MainWindow::connectSalesSignals()
{
    // Search functionality
    connect(ui->productSalesSearchEdit_2, &QLineEdit::textChanged,
            this, &MainWindow::on_productSalesSearchEdit_textChanged);
    connect(ui->salesSearchEdit, &QLineEdit::textChanged,
            this, &MainWindow::on_salesSearchEdit_textChanged);

    // Table interactions
    connect(ui->searchProductTable, &QTableWidget::cellClicked,
            this, &MainWindow::on_searchProductTable_cellClicked);
    connect(ui->selectedProductsTable, &QTableWidget::cellClicked,
            this, &MainWindow::on_selectedProductsTable_cellClicked);

    // Quantity controls

    // Action buttons
    connect(ui->sellProductsBtn, &QPushButton::clicked,
            this, &MainWindow::on_sellProductsBtn_clicked);
    connect(ui->clearSelectionBtn, &QPushButton::clicked,
            this, &MainWindow::on_clearSelectionBtn_clicked);
}

void MainWindow::integrateSalesDashboard()
{
    if (!m_dbHandler || !m_productManager || !m_salesManager) {
        qDebug() << "Required managers not initialized for SalesDashboard";
        return;
    }

    m_salesdashboard = new SalesDashboard(m_dbHandler, m_productManager, m_salesManager, this);

    // Add to UI if needed (assuming you have a container for it)
    // ui->salesDashboardContainer->addWidget(m_salesdashboard);
}

bool MainWindow::initializeSalesSystem()
{
    try {
        setupSalesManager();
        integrateSalesDashboard();
        return true;
    } catch (const std::exception& e) {
        qDebug() << "Failed to initialize sales system:" << e.what();
        return false;
    }
}

void MainWindow::showSalesDashboard()
{
    if (m_salesdashboard) {
        m_salesdashboard->refreshData();
        m_salesdashboard->show();
    }
}

// Refresh methods
void MainWindow::refreshSalesTable()
{
    if (m_salesManager && ui->salesTable) {
        m_salesManager->loadSales(ui->salesTable);
    }
}

void MainWindow::refreshProductSalesTable()
{
    if (m_productManager && ui->searchProductTable) {
        m_productManager->loadProducts(ui->searchProductTable);
    }
}

void MainWindow::refreshSelectedProductsTable()
{
    if (!ui->selectedProductsTable) return;

    ui->selectedProductsTable->setRowCount(0);

    for (int i = 0; i < m_selectedItems.size(); i++) {
        const SaleItem &item = m_selectedItems[i];
        int row = ui->selectedProductsTable->rowCount();
        ui->selectedProductsTable->insertRow(row);

        // Set item data
        ui->selectedProductsTable->setItem(row, 0, new QTableWidgetItem(item.productName));
        ui->selectedProductsTable->setItem(row, 1, new QTableWidgetItem(QString::number(item.unitPrice, 'f', 2)));
        ui->selectedProductsTable->setItem(row, 2, new QTableWidgetItem(QString::number(item.quantity)));
        ui->selectedProductsTable->setItem(row, 3, new QTableWidgetItem(QString::number(item.totalPrice, 'f', 2)));

        // Create remove button
        auto *removeBtn = new QPushButton("×", this);
        removeBtn->setStyleSheet("background-color: #e74c3c; color: white; font-weight: bold; padding: 5px;");
        removeBtn->setCursor(Qt::PointingHandCursor);

        // Connect with proper index capture
        connect(removeBtn, &QPushButton::clicked, [this, i]() {
            if (i < m_selectedItems.size()) {
                m_selectedItems.removeAt(i);
                refreshSelectedProductsTable();
                updateSalesTotals();
            }
        });

        ui->selectedProductsTable->setCellWidget(row, 4, removeBtn);
    }

    updateSalesTotals();
}

void MainWindow::updateSalesTotals()
{
    m_totalAmount = 0.0;
    for (const SaleItem &item : m_selectedItems) {
        m_totalAmount += item.totalPrice;
    }

    if (ui->labelTotalPrice) {
        ui->labelTotalPrice->setText(QString("%1 Rs.").arg(m_totalAmount, 0, 'f', 2));
    }
}

void MainWindow::addProductToSelection(const SaleItem &item)
{
    // Check stock availability
    if (item.available <= 0) {
        QMessageBox::warning(this, "Out of Stock",
                             QString("Product '%1' is currently out of stock.").arg(item.productName));
        return;
    }

    // Check if product already exists in selection
    for (int i = 0; i < m_selectedItems.size(); i++) {
        if (m_selectedItems[i].productId == item.productId) {
            auto &existingItem = m_selectedItems[i];
            if (existingItem.quantity < existingItem.available) {
                existingItem.quantity++;
                existingItem.totalPrice = existingItem.quantity * existingItem.unitPrice;
                refreshSelectedProductsTable();
                highlightSelectedProduct(i);
            } else {
                QMessageBox::warning(this, "Insufficient Stock",
                                     "Cannot add more of this product. Stock limit reached.");
            }
            return;
        }
    }

    // Add new product
    m_selectedItems.append(item);
    refreshSelectedProductsTable();
    highlightSelectedProduct(m_selectedItems.size() - 1);
}

void MainWindow::highlightSelectedProduct(int row)
{
    if (!ui->selectedProductsTable) return;

    // Clear all highlights first
    for (int i = 0; i < ui->selectedProductsTable->rowCount(); i++) {
        for (int j = 0; j < ui->selectedProductsTable->columnCount(); j++) {
            auto *item = ui->selectedProductsTable->item(i, j);
            if (item) {
                item->setBackground(i == row ? QColor("#436cfd") : Qt::transparent);
            }
        }
    }

    m_currentSelectedRow = row;
}

// Slot implementations
void MainWindow::on_productSalesSearchEdit_textChanged(const QString &text)
{
    if (!m_productManager || !ui->searchProductTable) return;

    if (text.isEmpty()) {
        refreshProductSalesTable();
    } else {
        m_productManager->searchProducts(ui->searchProductTable, text);
    }
}

void MainWindow::on_salesSearchEdit_textChanged(const QString &text)
{
    if (!m_salesManager || !ui->salesTable) return;

    m_salesManager->searchSales(ui->salesTable, text);
}

void MainWindow::on_searchProductTable_cellClicked(int row, int column)
{
    if (!ui->searchProductTable || row < 0 || row >= ui->searchProductTable->rowCount()) return;

    // Extract product data from table
    SaleItem item;
    item.productId = ui->searchProductTable->item(row, 0)->text().toInt();
    item.productName = ui->searchProductTable->item(row, 1)->text();
    item.unitPrice = ui->searchProductTable->item(row, 2)->text().toDouble();
    item.category = ui->searchProductTable->item(row, 3)->text();
    item.available = ui->searchProductTable->item(row, 4)->text().toInt();
    item.quantity = 1;
    item.totalPrice = item.unitPrice;

    addProductToSelection(item);
}

void MainWindow::on_selectedProductsTable_cellClicked(int row, int column)
{
    highlightSelectedProduct(row);
}

void MainWindow::on_addQtyBtn_clicked()
{
    if (m_currentSelectedRow >= 0 && m_currentSelectedRow < m_selectedItems.size()) {
        auto &item = m_selectedItems[m_currentSelectedRow];

        if (item.quantity < item.available) {
            item.quantity++;
            item.totalPrice = item.quantity * item.unitPrice;
            refreshSelectedProductsTable();
            highlightSelectedProduct(m_currentSelectedRow);
        } else {
            QMessageBox::warning(this, "Insufficient Stock",
                                 "Cannot add more of this product. Stock limit reached.");
        }
    }
}


void MainWindow::on_removeQtyBtn_clicked()
{
    if (m_currentSelectedRow >= 0 && m_currentSelectedRow < m_selectedItems.size()) {
        auto &item = m_selectedItems[m_currentSelectedRow];

        if (item.quantity > 1) {
            item.quantity--;
            item.totalPrice = item.quantity * item.unitPrice;
            refreshSelectedProductsTable();
            highlightSelectedProduct(m_currentSelectedRow);
        }
    }
}

void MainWindow::on_sellProductsBtn_clicked()
{
    if (m_selectedItems.isEmpty()) {
        QMessageBox::warning(this, "No Products Selected",
                             "Please select at least one product to complete the sale.");
        return;
    }

    if (!m_dbHandler || !m_salesManager) {
        QMessageBox::critical(this, "System Error",
                              "Sales system not properly initialized.");
        return;
    }

    int userId = m_dbHandler->getCurrentUserId();
    if (userId <= 0) {
        QMessageBox::critical(this, "Authentication Error",
                              "User information not available. Please log in again.");
        return;
    }

    // Process the sale
    if (m_salesManager->processSale(m_selectedItems, userId)) {
        QMessageBox::information(this, "Sale Completed",
                                 QString("Sale of %1 items totaling %2 Rs. has been successfully recorded.")
                                     .arg(m_selectedItems.size())
                                     .arg(m_totalAmount, 0, 'f', 2));

        // Clear selection after successful sale
        m_selectedItems.clear();
        m_currentSelectedRow = -1;
        refreshSelectedProductsTable();
        refreshSalesTable();
        refreshProductSalesTable(); // Refresh to show updated stock
        updateDashboard(); // Update main dashboard
    } else {
        QMessageBox::critical(this, "Sale Failed",
                              "There was an error processing the sale. Please try again.");
    }
}

void MainWindow::on_clearSelectionBtn_clicked()
{
    if (m_selectedItems.isEmpty()) return;

    auto reply = QMessageBox::question(this, "Clear Selection",
                                       "Are you sure you want to clear all selected products?",
                                       QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        m_selectedItems.clear();
        m_currentSelectedRow = -1;
        refreshSelectedProductsTable();
    }
}


// --- Other UI Slots ---
void MainWindow::on_cross_2_clicked() { // Example: A close button on a specific page
    if(m_dbHandler && ui->stackedWidget){
        if(m_dbHandler->isAdmin()){
            ui->stackedWidget->setCurrentIndex(3); // e.g., back to admin's product page
        } else {
            ui->stackedWidget->setCurrentIndex(9); // e.g., back to worker's stock page
        }
    }
}

void MainWindow::on_pushButton_clicked() { // Generic placeholder if this button exists
    qDebug() << "on_pushButton_clicked triggered - no specific action defined in this version.";
}
