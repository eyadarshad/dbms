#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>
#include <QColor>
#include <QRegularExpression>
#include <QLabel>
#include "debtmanager.h"
#include <QMessageBox>
#include <QDateTime>
#include "salesdashboard.h"
#include <QVBoxLayout>
#include "vendormanager.h"
#include "workermanager.h"
#include "salesdashboard.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    ,  m_dbHandler(new DatabaseHandler(this))
    , m_debtManager(new DebtManager(m_dbHandler, this))
    , m_productManager(new ProductManager(m_dbHandler, this))
    , m_vendorManager(new VendorManager(m_dbHandler, this))
    , m_workManager(new WorkerManager(m_dbHandler, this))
    , m_salesManager(new SalesManager(m_dbHandler, this))
{
    ui->setupUi(this);
    isDarkMode = true;
    updateDashboard();
    loginpage();
    // m_vendorManager = new VendorManager(m_dbHandler, this);
    // m_workManager = new WorkerManager(m_dbHandler, this);
    // m_stockManager = new StockManager(m_dbHandler, this);
    // Initialize SalesManager



    setupChart();
    setupCalculator();
    setupNavigation();
    QApplication::setStyle(QStyleFactory::create("Fusion"));  // Optional: To use the Fusion style
    QApplication::setPalette(QPalette(QPalette::WindowText, QColor(255, 255, 255)));  // Change text color globally

    // You can also use a stylesheet for more control
    qApp->setStyleSheet("QLabel { color: #ffffff; }");  // Set default text color for all QLabels to white

    // Connect theme buttons
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

    // Connect light mode buttons
    for (QPushButton* btn : lightModeButtons) {
        connect(btn, &QPushButton::clicked, this, &MainWindow::enableLightMode);
    }

    // Connect dark mode buttons
    for (QPushButton* btn : darkModeButtons) {
        connect(btn, &QPushButton::clicked, this, &MainWindow::enableDarkMode);
    }

    // Set initial UI elements
    ui->eyeButton->setIcon(QIcon(":/new/prefix1/images/view-Stroke-Rounded.png"));
    ui->password_login->setEchoMode(QLineEdit::Password);

    m_dbHandler = new DatabaseHandler(this);
    if (!m_dbHandler->connectToDatabase()) {
        QMessageBox::critical(this, "Database Error",
                              "Failed to connect to database. Please check your connection.");
        // You might want to disable certain features or exit
    }

    m_debtManager = new DebtManager(m_dbHandler, this);
    connect(m_debtManager, &DebtManager::debtorsUpdated, this, &MainWindow::onDebtorsUpdated);
    connect(m_productManager, &ProductManager::productsUpdated, this, &MainWindow::onProductsUpdated);
    connect(m_vendorManager, &VendorManager::vendorsUpdated, this, &MainWindow::onVendorsUpdated);
    connect(m_workManager, &WorkerManager::workersUpdated, this, &MainWindow::onWorkersUpdated);
    // connect(m_stockManager, &StockManager::stockUpdated, this, &MainWindow::onStockUpdated);
    connect(m_salesManager, &SalesManager::salesUpdated, this, &MainWindow::onSalesUpdated);

    // Add these after setupProductManager() and setupDebtManager() calls

    setupDebtManager();
    setupProductManager();
    setupVendorManager();
    setupWorkerManager();
    setupSalesManager();
    setupStockManager();
    integrateSalesDashboard();
     initializeSalesSystem();
    // Initialize sales-related variables


    // Setup sales management



}

MainWindow::~MainWindow() {
    delete ui;
    // Clean up sales manager
    delete m_salesManager;
}

void MainWindow::setupChart() {
    QLineSeries *series = new QLineSeries();
    *series << QPointF(0, 6) << QPointF(2, 4) << QPointF(3, 8)
            << QPointF(7, 4) << QPointF(10, 5) << QPointF(11, 1)
            << QPointF(13, 3) << QPointF(17, 6) << QPointF(18, 3) << QPointF(20, 2);

    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->createDefaultAxes();
    chart->setTitle("");
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setParent(ui->horizontalFrame);
    chartView->setGeometry(0, 0, 431, 241);
}

void MainWindow::setupCalculator() {
    ui->gridLayout_2->addWidget(ui->display_2, 0, 0, 1, 3);

    // Add backspace button
    QPushButton *backspaceBtn = new QPushButton("⌫");
    backspaceBtn->setFixedSize(60, 60);
    ui->gridLayout_2->addWidget(backspaceBtn, 0, 3);
    connect(backspaceBtn, &QPushButton::clicked, this, [=]() {
        QString text = ui->display_2->text();
        if (!text.isEmpty())
            ui->display_2->setText(text.left(text.length() - 1));
    });

    // Setup calculator buttons
    QStringList buttons = {
        "7", "8", "9", "C",
        "4", "5", "6", "*",
        "1", "2", "3", "-",
        "0", ".", "=", "+"
    };

    int pos = 0;
    for (int row = 1; row <= 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            QString label = buttons[pos++];
            QPushButton *btn = new QPushButton(label);
            btn->setObjectName("btn" + label);
            btn->setFixedSize(60, 60);
            ui->gridLayout_2->addWidget(btn, row, col);

            connect(btn, &QPushButton::clicked, this, [=]() {
                QString val = btn->text();
                if (val == "=") {
                    QJSEngine engine;
                    QJSValue result = engine.evaluate(ui->display_2->text());
                    ui->display_2->setText(result.toString());
                } else if (val == "C") {
                    ui->display_2->clear();
                } else {
                    ui->display_2->setText(ui->display_2->text() + val);
                }
            });
        }
    }
}

void MainWindow::connectPageButton(QPushButton *button, int index) {
    if (button)
        connect(button, &QPushButton::clicked, this, [=]() {
            ui->stackedWidget->setCurrentIndex(index);
        });
}

void MainWindow::setupNavigation() {
    QMap<QPushButton*, int> navMap = {
        {ui->page1, 1}, {ui->page2, 2}, {ui->page3, 3}, {ui->page4, 4},
        {ui->page5, 5}, {ui->page6, 6}, {ui->page7, 7}, {ui->page21, 1},
        {ui->page22, 2}, {ui->page23, 3}, {ui->page24, 4}, {ui->page25, 5},
        {ui->page26, 6}, {ui->page27, 7}, {ui->page31, 1}, {ui->page32, 2},
        {ui->page33, 3}, {ui->page34, 4}, {ui->page35, 5}, {ui->page36, 6},
        {ui->page37, 7}, {ui->page41, 1}, {ui->page42, 2}, {ui->page43, 3},
        {ui->page44, 4}, {ui->page45, 5}, {ui->page46, 6}, {ui->page47, 7},
        {ui->page51, 1}, {ui->page52, 2}, {ui->page53, 3}, {ui->page54, 4},
        {ui->page55, 5}, {ui->page56, 6}, {ui->page57, 7}, {ui->page61, 1},
        {ui->page62, 2}, {ui->page63, 3}, {ui->page64, 4}, {ui->page65, 5},
        {ui->page66, 6}, {ui->page67, 7}, {ui->page71, 1}, {ui->page72, 2},
        {ui->page73, 3}, {ui->page74, 4}, {ui->page75, 5}, {ui->page76, 6},
        {ui->page77, 7}, {ui->pagew1, 8}, {ui->pagew2, 9}, {ui->pagew3, 10},
        {ui->pagew4, 11}, {ui->pagew21, 8}, {ui->pagew22, 9}, {ui->pagew23, 10},
        {ui->pagew24, 11}, {ui->pagew31, 8}, {ui->pagew32, 9}, {ui->pagew33, 10},
        {ui->pagew34, 11}, {ui->pages1_2, 8}, {ui->pages2_2, 9}, {ui->pages3_2, 10},
        {ui->pages4_2, 11}, {ui->btnlogout_8, 0}, {ui->btnlogout_2, 0},
        {ui->btnlogout_3, 0}, {ui->btnlogout_4, 0}, {ui->btnlogout_5, 0},
        {ui->btnlogout_6, 0}, {ui->btnlogout_7, 0},
        {ui->btnlogout_9, 0}, {ui->btnlogout_10, 0}, {ui->btnlogout_11, 0},
        {ui->btnlogout_12, 0}, {ui->cross, 7},
        {ui->addProductBtn, 14}, {ui->addWorkerBtn, 12}, {ui->addDebtorBtn, 13}, {ui->cross_3, 2},
    {ui->addProductBtn_4, 14},{ui->addVendorBtn_2, 5},{ui->addVendorBtn, 15}, {ui->addWorkerBtn_2,7}, {ui->addDebtorBtn_2, 2}
    };


    for (auto it = navMap.begin(); it != navMap.end(); ++it) {
        connectPageButton(it.key(), it.value());
    }
}

void MainWindow::on_pushButton_clicked() {
    // Empty implementation as placeholder
}


void MainWindow::showDarkMessageBox(const QString &title, const QString &message) {
    QMessageBox msgBox;
    msgBox.setWindowTitle(title);
    msgBox.setText(message);
    msgBox.setIcon(QMessageBox::Warning);

    msgBox.setStyleSheet(
        "QMessageBox { background-color: #2b2b2b; color: black; }"
        "QPushButton { background-color: #444; color: black; padding: 10px; width:10px; border-radius: 10px; }"
        "QPushButton:hover { background-color: #666; }"
        );

    msgBox.exec();
}

void MainWindow::on_eyeButton_clicked() {
    passwordVisible = !passwordVisible;

    if (passwordVisible) {
        ui->password_login->setEchoMode(QLineEdit::Normal);
        QString iconPath = isDarkMode ? ":/new/prefix1/images/eyebrow (1).png" : ":/new/prefix1/images/eyebrow.png";
        ui->eyeButton->setIcon(QIcon(iconPath));
        ui->eyeButton->setProperty("iconSource", iconPath);
    } else {
        ui->password_login->setEchoMode(QLineEdit::Password);
        QString iconPath = isDarkMode ? ":/new/prefix1/images/view-Stroke-Rounded.png" : ":/new/prefix1/images/view-Stroke-Rounded (1).png";
        ui->eyeButton->setIcon(QIcon(iconPath));
        ui->eyeButton->setProperty("iconSource", iconPath);
    }
}


QString MainWindow::getAlternateThemePath(const QString &currentPath) {
    for (auto it = iconPairMappings.begin(); it != iconPairMappings.end(); ++it) {
        QString darkPattern = it.key();
        QString lightPattern = it.value();

        QString copy = currentPath;

        if (currentPath.contains("dark")) {
            return copy.replace(darkPattern, lightPattern);
        } else {
            return copy.replace(lightPattern, darkPattern);
        }
    }
    return QString();
}

void MainWindow::updateAllIcons() {
    // Update label icons
    for (QLabel* label : findChildren<QLabel*>()) {
        if (label->pixmap().isNull()) continue;
        QString currentPath = label->property("iconSource").toString();
        if (currentPath.isEmpty()) continue;

        QString newPath = getAlternateThemePath(currentPath);
        if (!newPath.isEmpty()) {
            label->setPixmap(QPixmap(newPath));
            label->setProperty("iconSource", newPath);
        }
    }

    // Update button icons
    for (QPushButton* button : findChildren<QPushButton*>()) {
        if (button->icon().isNull()) continue;
        QString currentPath = button->property("iconSource").toString();
        if (currentPath.isEmpty()) continue;

        QString newPath = getAlternateThemePath(currentPath);
        if (!newPath.isEmpty()) {
            button->setIcon(QIcon(newPath));
            button->setProperty("iconSource", newPath);
        }
    }
}

void MainWindow::initializeColorMappings() {
    // Clear existing mappings
    colorMappings.clear();

    // REFINED LIGHT MODE COLOR PALETTE
    // Core Background Colors
    colorMappings["#0a0a0a"] = "#ffffff";    // Pure white for main backgrounds
    colorMappings["#121212"] = "#f8fafc";    // Barely noticeable blue tint
    colorMappings["#181818"] = "#f1f5f9";    // Soft blue-gray for containers
    colorMappings["#262626"] = "#e2e8f0";    // Light slate for form fields
    colorMappings["#3a3a3a"] = "#cbd5e1";    // Light blue-gray for borders
    colorMappings["#ffffff"] = "#0a0a0a";
    // UI Element Colors
    colorMappings["#565656"] = "#94a3b8";    // Medium slate for UI elements
    colorMappings["#adadad"] = "#64748b";    // Medium-dark slate
    colorMappings["#626262"] = "#475569";    // Dark slate for secondary text

    // Text Colors
    colorMappings["#fcfcfc"] = "#1e293b";    // Deep blue-slate for primary text
    colorMappings["white"] = "#0f172a";      // Near-black with blue undertone
    colorMappings["#e0e4e4"] = "#334155";    // Dark slate for paragraphs
    colorMappings["#494949"] = "#1e293b";    // Deep slate for important text

    // Accent Colors
    colorMappings["#436cfd"] = "#2563eb";    // Royal blue for primary actions
    colorMappings["#0078d7"] = "#3b82f6";    // Medium blue for secondary actions

    // Semantic Colors
    colorMappings["#4fc3f7"] = "#0ea5e9";    // Sky blue for information elements
    colorMappings["#66bb6a"] = "#10b981";    // Emerald green for success states
    colorMappings["#ff7043"] = "#f59e0b";    // Amber for warnings
    colorMappings["#ef5350"] = "#ef4444";    // Red for errors

    // Additional UI Colors
    colorMappings["#2d3748"] = "#e2e8f0";    // Light gray for card backgrounds
    colorMappings["#4a5568"] = "#cbd5e1";    // Light blue-gray for inactive tabs
    colorMappings["#718096"] = "#475569";    // Slate for borders on hover
    colorMappings["#a0aec0"] = "#64748b";    // Medium slate for disabled text
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
    return QColor(colorStr);
}

QString MainWindow::colorToString(const QColor &color, const QString &originalFormat) {
    if (originalFormat.startsWith("#")) {
        return color.name();
    } else if (originalFormat.startsWith("rgb(")) {
        return QString("rgb(%1,%2,%3)").arg(color.red()).arg(color.green()).arg(color.blue());
    } else if (originalFormat.startsWith("rgba(")) {
        return QString("rgba(%1,%2,%3,%4)").arg(color.red()).arg(color.green())
        .arg(color.blue()).arg(color.alpha());
    }
    return color.name();
}

QString MainWindow::applyLightModeColor(const QString &originalColor) {
    QString cleanColor = originalColor.trimmed().toLower();

    if (colorMappings.contains(cleanColor)) {
        return colorMappings[cleanColor];
    }

    QColor color = stringToColor(cleanColor);
    if (color.isValid()) {
        int average = (color.red() + color.green() + color.blue()) / 3;
        if (average < 128) {
            QColor lightColor;
            if (color.hue() == -1) {  // For grays
                int lightness = 255 - color.value();
                lightColor = QColor(lightness, lightness, lightness);
            } else {
                lightColor = QColor::fromHsv(
                    color.hue(),
                    qMax(0, color.saturation() - 40),
                    qMin(255, color.value() + 150)
                    );
            }
            return colorToString(lightColor, cleanColor);
        }
    }
    return QString();
}

QString MainWindow::processStyleSheetForLightMode(const QString &styleSheet) {
    QString result = styleSheet;
    QStringList colorProperties = {
        "background", "background-color", "color", "border", "border-color",
        "gridline-color", "selection-background-color", "selection-color"
    };

    for (const QString &prop : colorProperties) {
        QRegularExpression colorRegex(prop + "\\s*:\\s*([^;]+)");
        QRegularExpressionMatchIterator matches = colorRegex.globalMatch(result);

        while (matches.hasNext()) {
            QRegularExpressionMatch match = matches.next();
            QString colorStr = match.captured(1).trimmed();

            // Skip non-color values
            if (colorStr == "transparent" || colorStr == "none" ||
                colorStr.contains("url") || colorStr.contains("gradient")) {
                continue;
            }

            // Apply light mode color mapping
            QString newColorStr = applyLightModeColor(colorStr);
            if (!newColorStr.isEmpty()) {
                result.replace(match.capturedStart(1), match.capturedLength(1), newColorStr);
            }
        }
    }
    return result;
}

void MainWindow::loginpage(){
    ui->stackedWidget->setCurrentIndex(0);
}



void MainWindow::applyLightModeToTable(QTableWidget* table) {
    // Store original table settings if not already stored
    if (!originalTableSettings.contains(table)) {
        TableSettings settings;

        // Header settings
        for (int i = 0; i < table->horizontalHeader()->count(); ++i) {
            QTableWidgetItem* headerItem = table->horizontalHeaderItem(i);
            if (headerItem) {
                settings.headerBackgrounds[i] = headerItem->background();
                settings.headerForegrounds[i] = headerItem->foreground();
            }
        }

        // Item settings
        for (int row = 0; row < table->rowCount(); ++row) {
            for (int col = 0; col < table->columnCount(); ++col) {
                QTableWidgetItem* item = table->item(row, col);
                if (item) {
                    settings.itemBackgrounds[QPair<int, int>(row, col)] = item->background();
                    settings.itemForegrounds[QPair<int, int>(row, col)] = item->foreground();
                }
            }
        }

        originalTableSettings[table] = settings;
    }

    // Apply light mode styles
    table->horizontalHeader()->setStyleSheet(
        "QHeaderView::section {"
        "    background-color: #2563eb;"
        "    color: white;"
        "    padding: 6px;"
        "    font-weight: bold;"
        "    border: none;"
        "    border-right: 1px solid #1e40af;"
        "    border-bottom: 2px solid #1e40af;"
        "}"
        );

    table->verticalHeader()->setStyleSheet(
        "QHeaderView::section {"
        "    background-color: #e2e8f0;"
        "    color: #334155;"
        "    padding: 4px;"
        "    border: none;"
        "    border-bottom: 1px solid #cbd5e1;"
        "    border-right: 2px solid #cbd5e1;"
        "}"
        );

    table->setShowGrid(true);
    table->setGridStyle(Qt::SolidLine);
    table->setAlternatingRowColors(true);

    table->setStyleSheet(table->styleSheet() +
                         "QTableWidget {"
                         "    gridline-color: #e2e8f0;"
                         "    background-color: white;"
                         "    selection-background-color: #bfdbfe;"
                         "    selection-color: #1e3a8a;"
                         "    border: 1px solid #cbd5e1;"
                         "    border-radius: 4px;"
                         "}"
                         "QTableWidget::item {"
                         "    padding: 4px;"
                         "    border-bottom: 1px solid #f1f5f9;"
                         "}"
                         "QTableWidget::item:selected {"
                         "    background-color: #bfdbfe;"
                         "}"
                         );

    // Apply alternating row colors
    for (int row = 0; row < table->rowCount(); ++row) {
        QColor rowBgColor = (row % 2 == 0) ? QColor("#ffffff") : QColor("#f8fafc");

        for (int col = 0; col < table->columnCount(); ++col) {
            QTableWidgetItem* item = table->item(row, col);
            if (item) {
                item->setBackground(QBrush(rowBgColor));
                item->setForeground(QBrush(QColor("#334155")));
            }
        }
    }
}

void MainWindow::applyLightModeToAllWidgets() {
    initializeColorMappings();
    originalStylesheets.clear();

    // Process all widgets
    for (QWidget* widget : findChildren<QWidget*>()) {
        // Store original stylesheet
        originalStylesheets[widget] = widget->styleSheet();

        // Apply light mode colors
        widget->setStyleSheet(processStyleSheetForLightMode(widget->styleSheet()));

        // Handle specific widget types
        if (QTableWidget* table = qobject_cast<QTableWidget*>(widget)) {
            applyLightModeToTable(table);
        }
    }

    // Store and process main window stylesheet
    originalMainStylesheet = styleSheet();
    setStyleSheet(processStyleSheetForLightMode(styleSheet()));




    QString scrollbarStyle =
        "QScrollBar:vertical {"
        "    background: #f1f5f9;"
        "    width: 10px;"
        "    margin: 0px 0px 0px 0px;"
        "    border-radius: 5px;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background: #94a3b8;"
        "    min-height: 20px;"
        "    border-radius: 5px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "    background: #64748b;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "    height: 0px;"
        "}"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
        "    background: none;"
        "}"
        "QScrollBar:horizontal {"
        "    background: #f1f5f9;"
        "    height: 10px;"
        "    margin: 0px 0px 0px 0px;"
        "    border-radius: 5px;"
        "}"
        "QScrollBar::handle:horizontal {"
        "    background: #94a3b8;"
        "    min-width: 20px;"
        "    border-radius: 5px;"
        "}"
        "QScrollBar::handle:horizontal:hover {"
        "    background: #64748b;"
        "}"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {"
        "    width: 0px;"
        "}"
        "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {"
        "    background: none;"
        "}";

    // Apply scrollbar style to all scrollable widgets
    for (QAbstractScrollArea* scrollArea : findChildren<QAbstractScrollArea*>()) {
        scrollArea->setStyleSheet(scrollArea->styleSheet() + scrollbarStyle);
    }

    // Update UI for light mode
    updateAllIcons();
}

void MainWindow::restoreDarkModeToAllWidgets() {
    // Restore original stylesheets
    for (auto it = originalStylesheets.begin(); it != originalStylesheets.end(); ++it) {
        if (it.key()) {
            it.key()->setStyleSheet(it.value());
        }
    }

    // Restore main window stylesheet
    setStyleSheet(originalMainStylesheet);

    // Restore table widget styles
    for (auto it = originalTableSettings.begin(); it != originalTableSettings.end(); ++it) {
        QTableWidget* table = it.key();
        if (!table) continue;

        TableSettings& settings = it.value();

        // Restore header settings
        for (int i = 0; i < table->horizontalHeader()->count(); ++i) {
            QTableWidgetItem* headerItem = table->horizontalHeaderItem(i);
            if (headerItem && settings.headerBackgrounds.contains(i)) {
                headerItem->setBackground(settings.headerBackgrounds[i]);
                headerItem->setForeground(settings.headerForegrounds[i]);
            }
        }

        // Restore item settings
        for (int row = 0; row < table->rowCount(); ++row) {
            for (int col = 0; col < table->columnCount(); ++col) {
                QTableWidgetItem* item = table->item(row, col);
                QPair<int, int> key(row, col);
                if (item && settings.itemBackgrounds.contains(key)) {
                    item->setBackground(settings.itemBackgrounds[key]);
                    item->setForeground(settings.itemForegrounds[key]);
                }
            }
        }
    }

    // Update UI for dark mode
    updateAllIcons();
}

void MainWindow::enableLightMode() {
    if (isDarkMode) {

            // White text for dark mode
        // Start transition animation
        QPropertyAnimation* opacityAnim = new QPropertyAnimation(this, "windowOpacity");
        opacityAnim->setDuration(250);
        opacityAnim->setStartValue(1.0);
        opacityAnim->setEndValue(0.8);
        opacityAnim->setEasingCurve(QEasingCurve::InOutQuad);

        connect(opacityAnim, &QPropertyAnimation::finished, this, [=]() {
            // Set light mode
            isDarkMode = false;
            applyLightModeToAllWidgets();
            if (isDarkMode) {
                qApp->setStyleSheet("QLabel { color: #FFFFFF; }");  // White text for dark mode
            } else {
                qApp->setStyleSheet("QLabel { color: #000000; }");  // Black text for light mode
            }

            // Transition back to full opacity
            QPropertyAnimation* endAnim = new QPropertyAnimation(this, "windowOpacity");
            endAnim->setDuration(250);
            endAnim->setStartValue(0.8);
            endAnim->setEndValue(1.0);
            endAnim->setEasingCurve(QEasingCurve::InOutQuad);
            endAnim->start(QAbstractAnimation::DeleteWhenStopped);
        });

        opacityAnim->start(QAbstractAnimation::DeleteWhenStopped);
    }

}

void MainWindow::enableDarkMode() {
    if (!isDarkMode) {
        // Start transition animation
        QPropertyAnimation* opacityAnim = new QPropertyAnimation(this, "windowOpacity");
        opacityAnim->setDuration(250);
        opacityAnim->setStartValue(1.0);
        opacityAnim->setEndValue(0.8);
        opacityAnim->setEasingCurve(QEasingCurve::InOutQuad);

        connect(opacityAnim, &QPropertyAnimation::finished, this, [=]() {
            // Set dark mode
            isDarkMode = true;
            restoreDarkModeToAllWidgets();
            if (isDarkMode) {
                qApp->setStyleSheet("QLabel { color: #FFFFFF; }");  // White text for dark mode
            } else {
                qApp->setStyleSheet("QLabel { color: #000000; }");  // Black text for light mode
            }
            // Transition back to full opacity
            QPropertyAnimation* endAnim = new QPropertyAnimation(this, "windowOpacity");
            endAnim->setDuration(250);
            endAnim->setStartValue(0.8);
            endAnim->setEndValue(1.0);
            endAnim->setEasingCurve(QEasingCurve::InOutQuad);
            endAnim->start(QAbstractAnimation::DeleteWhenStopped);
        });

        opacityAnim->start(QAbstractAnimation::DeleteWhenStopped);
    }
}
void MainWindow::on_loginbtn_clicked()
{
    QString username = ui->username_login->text();
    QString password = ui->password_login->text();

    if (m_dbHandler->login(username, password)) {
        // Login successful - now check if admin or worker
        if (m_dbHandler->isAdmin()) {
            ui->stackedWidget->setCurrentIndex(1); // Admin dashboard
        } else {
            ui->stackedWidget->setCurrentIndex(8); // Worker dashboard
        }


    } else {
        QMessageBox::warning(this, "Login Failed",
                             "Invalid username or password. Please try again.");
    }
}

void MainWindow::logoutUser()
{
    m_dbHandler->logout();
    ui->stackedWidget->setCurrentIndex(0); // Back to login page
    ui->username_login->clear();
    ui->password_login->clear();
}

// Optimized MainWindow implementation for debt and product management

void MainWindow::setupDebtManager()
{
    m_debtManager = new DebtManager(m_dbHandler, this);
    connect(m_debtManager, &DebtManager::debtorsUpdated, this, &MainWindow::onDebtorsUpdated);

    setupTableWidget(ui->debtorsTable, {"ID", "Name", "Contact", "Email", "Address", "Amount", "Date"});
    refreshDebtorTable();
}

void MainWindow::setupProductManager()
{
    m_productManager = new ProductManager(m_dbHandler, this);
    connect(m_productManager, &ProductManager::productsUpdated, this, &MainWindow::onProductsUpdated);

    setupTableWidget(ui->productsTable, {"ID", "Name", "Price", "Category", "Quantity", "Added At"});
     setupTableWidget(ui->productsTable_2, {"ID", "Name", "Price", "Category", "Quantity", "Added At"});
    refreshProductTable();

    // Setup category combo
    ui->categoryCombo->addItems({"Electronics", "Clothing", "Food", "Beverages", "Household", "Other"});
}

// DEBT MANAGEMENT
void MainWindow::refreshDebtorTable()
{
    if (m_dbHandler->isConnected()) m_debtManager->loadDebtors(ui->debtorsTable);
}

void MainWindow::on_addDebtorBtn_clicked()
{
    clearDebtorForm();
    ui->stackedWidget->setCurrentIndex(13);
}

void MainWindow::on_addDebtorBtn_2_clicked()
{
    auto [name, contact, address, amount, date] = getDebtorFormData();

    if (!validateDebtorInput(name, contact, address, amount)) return;

    if (m_debtManager->addDebtor(name, contact, "", address, amount, date)) {
        showSuccess("Debtor added successfully.");
        ui->stackedWidget->setCurrentIndex(2);
        refreshDebtorTable();
    } else {
        showError("Failed to add debtor. Please try again.");
    }
}

void MainWindow::on_removeDebtorBtn_clicked()
{
    int row = ui->debtorsTable->currentRow();
    if (row < 0) {
        showWarning("Please select a debtor to remove.");
        return;
    }

    int id = ui->debtorsTable->item(row, 0)->text().toInt();
    QString name = ui->debtorsTable->item(row, 1)->text();

    if (confirmRemoval("debtor", name) && m_debtManager->removeDebtor(id)) {
        showSuccess("Debtor removed successfully.");
        refreshDebtorTable();
    } else if (!confirmRemoval("debtor", name)) {
        return;
    } else {
        showError("Failed to remove debtor.");
    }
}

void MainWindow::on_debtorSearchEdit_textChanged(const QString &text)
{
    if (!m_dbHandler->isConnected()) return;

    text.isEmpty() ? refreshDebtorTable() : m_debtManager->searchDebtors(ui->debtorsTable, text);
}

// PRODUCT MANAGEMENT
void MainWindow::refreshProductTable()
{
    if (m_dbHandler->isConnected()){
        m_productManager->loadProducts(ui->productsTable);
        m_productManager->loadProducts(ui->productsTable_2);
    }
}

void MainWindow::on_addProductBtn_clicked()
{
    clearProductForm();
    ui->stackedWidget->setCurrentIndex(14);
}
void MainWindow::on_addProductBtn_4_clicked()
{
    clearProductForm();
    ui->stackedWidget->setCurrentIndex(14);
}

void MainWindow::on_addProductBtn_2_clicked()
{
    auto [name, price, category, quantity, date] = getProductFormData();

    if (!validateProductInput(name, price, quantity)) return;

    if (m_productManager->addProduct(name, price, category, quantity, date)) {
        clearProductForm();
        showSuccessWithOk("Information saved successfully");
    } else {
        showError("Failed to add product.");
    }
}

void MainWindow::on_removeProductBtn_clicked()
{
    int row = ui->productsTable->currentRow();
    if (row < 0) {
        showError("Please select a product to remove!");
        return;
    }

    int id = ui->productsTable->item(row, 0)->text().toInt();
    QString name = ui->productsTable->item(row, 1)->text();

    if (confirmRemoval("product", name) && m_productManager->removeProduct(id)) {
        showSuccess("Product removed successfully.");
        refreshProductTable();
    } else if (!confirmRemoval("product", name)) {
        return;
    } else {
        showError("Failed to remove product!");
    }
}

void MainWindow::on_productSearchEdit_textChanged(const QString &text)
{
    text.isEmpty() ? refreshProductTable() : m_productManager->searchProducts(ui->productsTable, text);
}
void MainWindow::on_workerProductSearchEdit_textChanged(const QString &text)
{
    text.isEmpty() ? refreshProductTable() : m_productManager->searchProducts(ui->productsTable_2, text);
}

// EVENT HANDLERS
void MainWindow::onDebtorsUpdated()
{
    refreshDebtorTable();
    updateDashboard();
}

void MainWindow::onProductsUpdated()
{
    refreshProductTable();
    updateDashboard();
}

// HELPER METHODS
void MainWindow::setupTableWidget(QTableWidget *table, const QStringList &headers)
{
    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);
}

std::tuple<QString, QString, QString, double, QDate> MainWindow::getDebtorFormData()
{
    return {ui->debtorNameEdit->text().trimmed(),
            ui->debtorContactEdit->text().trimmed(),
            ui->debtorAddressEdit->text().trimmed(),
            ui->debtorAmountEdit->text().toDouble(),
            ui->dateEdit->date()};
}

std::tuple<QString, double, QString, int, QDate> MainWindow::getProductFormData()
{
    return {ui->productNameEdit->text(),
            ui->priceEdit->text().toDouble(),
            ui->categoryCombo->currentText(),
            ui->quantityEdit->text().toInt(),
            ui->dateEdit_2->date()};
}

bool MainWindow::validateDebtorInput(const QString &name, const QString &contact,
                                     const QString &address, double amount)
{
    if (name.isEmpty() || contact.isEmpty() || address.isEmpty() || amount <= 0) {
        showWarning("Please fill in all required fields.");
        return false;
    }
    return true;
}

bool MainWindow::validateProductInput(const QString &name, double price, int quantity)
{
    if (name.isEmpty() || price <= 0 || quantity < 0) {
        showError("Please enter valid product details.");
        return false;
    }
    return true;
}

void MainWindow::clearDebtorForm()
{
    ui->debtorNameEdit->clear();
    ui->debtorContactEdit->clear();
    ui->debtorAddressEdit->clear();
    ui->debtorAmountEdit->clear();
    ui->dateEdit->setDate(QDate::currentDate());
}

void MainWindow::clearProductForm()
{
    ui->productNameEdit->clear();
    ui->priceEdit->clear();
    ui->quantityEdit->clear();
    ui->categoryCombo->setCurrentIndex(0);
    ui->dateEdit_2->setDate(QDate::currentDate());
}

bool MainWindow::confirmRemoval(const QString &type, const QString &name)
{
    return QMessageBox::question(this, "Confirm Removal",
                                 QString("Are you sure you want to remove %1: %2?").arg(type, name),
                                 QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes;
}

void MainWindow::showSuccess(const QString &message)
{
    QMessageBox::information(this, "Success", message);
}

void MainWindow::showError(const QString &message)
{
    QMessageBox::critical(this, "Error", message);
}

void MainWindow::showWarning(const QString &message)
{
    QMessageBox::warning(this, "Warning", message);
}

bool MainWindow::showSuccessWithOk(const QString &message)
{
    QMessageBox msgBox;
    msgBox.setText(message);
    msgBox.setWindowTitle("Success");
    msgBox.setStandardButtons(QMessageBox::Ok);
    return msgBox.exec() == QMessageBox::Ok;
}
// Add these improved implementations to your MainWindow class
void MainWindow::setupVendorManager()
{
    m_vendorManager = new VendorManager(m_dbHandler, this);
    connect(m_vendorManager, &VendorManager::vendorsUpdated, this, &MainWindow::onVendorsUpdated);

    setupTableHeaders(ui->vendorsTable, {"ID", "Name", "Contact", "Address", "Payment", "Date of Supply"});
    refreshVendorTable();
}

void MainWindow::setupWorkerManager()
{
    m_workManager = new WorkerManager(m_dbHandler, this);
    connect(m_workManager, &WorkerManager::workersUpdated, this, &MainWindow::onWorkersUpdated);

    setupTableHeaders(ui->workersTable, {"ID", "Name", "Contact", "Email", "Status", "Salary", "Date of Joining"});
    refreshWorkerTable();

    // Setup status combo box
    ui->statusCombo->clear();
    ui->statusCombo->addItems({"Active", "On Leave", "Terminated", "Suspended", "Part-time"});
}

// Generic helper method to reduce code duplication
void MainWindow::setupTableHeaders(QTableWidget *table, const QStringList &headers)
{
    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);
}

void MainWindow::refreshVendorTable()
{
    if (m_dbHandler->isConnected() && m_vendorManager) {
        m_vendorManager->loadVendors(ui->vendorsTable);
    }
}

void MainWindow::refreshWorkerTable()
{
    if (m_dbHandler->isConnected() && m_workManager) {
        m_workManager->loadWorkers(ui->workersTable);
    }
}

void MainWindow::onVendorsUpdated()
{
    refreshVendorTable();
    updateDashboard();
}

void MainWindow::onWorkersUpdated()
{
    refreshWorkerTable();
    updateDashboard();
}

// Vendor management slots - significantly reduced
void MainWindow::on_addVendorBtn_clicked()
{
    clearForm({ui->vendorNameEdit, ui->vendorContactEdit, ui->vendorAddressEdit, ui->vendorPaymentEdit});
    ui->stackedWidget->setCurrentIndex(15);
}

void MainWindow::on_addVendorBtn_2_clicked()
{
    auto [name, contact, address, cashStr] = getFormData({ui->vendorNameEdit, ui->vendorContactEdit,
                                                          ui->vendorAddressEdit, ui->vendorPaymentEdit});

    if (!validateInput({name, contact, address, cashStr})) {
        showDarkMessageBox("Error", "All fields are required!");
        return;
    }

    if (m_vendorManager->addVendor(name, address, contact, cashStr.toDouble(), ui->dateEdit_4->date())) {
        clearForm({ui->vendorNameEdit, ui->vendorContactEdit, ui->vendorAddressEdit, ui->vendorPaymentEdit});
        ui->stackedWidget->setCurrentIndex(4);
        showDarkMessageBox("Success", "Vendor added successfully!");
    } else {
        showDarkMessageBox("Error", "Failed to add vendor!");
    }
}

void MainWindow::on_removeVendorBtn_clicked()
{
    if (auto id = getSelectedId(ui->vendorsTable, "vendor")) {
        if (m_vendorManager->removeVendor(*id)) {
            showDarkMessageBox("Success", "Vendor removed successfully!");
        } else {
            showDarkMessageBox("Error", "Failed to remove vendor!");
        }
    }
}

void MainWindow::on_vendorSearchEdit_textChanged(const QString &searchText)
{
    if (m_vendorManager) {
        m_vendorManager->searchVendors(ui->vendorsTable, searchText);
    }
}

// Worker management slots - significantly reduced
void MainWindow::on_addWorkerBtn_clicked()
{
    clearForm({ui->workerNameEdit, ui->workerContactEdit, ui->workerEmailEdit, ui->workerSalaryEdit});
    ui->statusCombo->setCurrentIndex(0);
    ui->dateEdit_3->setDate(QDate::currentDate());
    ui->stackedWidget->setCurrentIndex(12);
}

void MainWindow::on_addWorkerBtn_2_clicked()
{
    auto [name, contact, email, salaryStr] = getFormData({ui->workerNameEdit, ui->workerContactEdit,
                                                          ui->workerEmailEdit, ui->workerSalaryEdit});

    if (!validateInput({name, contact, salaryStr})) {
        showDarkMessageBox("Error", "All fields are required!");
        return;
    }

    if (m_workManager->addWorker(name, contact, email, ui->statusCombo->currentText(),
                                 salaryStr.toDouble(), ui->dateEdit_3->date())) {
        clearForm({ui->workerNameEdit, ui->workerContactEdit, ui->workerSalaryEdit});
        ui->stackedWidget->setCurrentIndex(7);
        showDarkMessageBox("Success", "Worker added successfully!");
    } else {
        showDarkMessageBox("Error", "Failed to add worker!");
    }
}

void MainWindow::on_removeWorkerBtn_clicked()
{
    if (auto id = getSelectedId(ui->workersTable, "worker")) {
        if (m_workManager->removeWorker(*id)) {
            showDarkMessageBox("Success", "Worker removed successfully!");
        } else {
            showDarkMessageBox("Error", "Failed to remove worker!");
        }
    }
}

void MainWindow::on_workerSearchEdit_textChanged(const QString &searchText)
{
    if (m_workManager) {
        m_workManager->searchWorkers(ui->workersTable, searchText);
    }
}

// Generic helper methods to reduce code duplication
void MainWindow::clearForm(const QList<QLineEdit*> &fields)
{
    for (auto field : fields) {
        field->clear();
    }
}

std::tuple<QString, QString, QString, QString> MainWindow::getFormData(const QList<QLineEdit*> &fields)
{
    QStringList data;
    for (auto field : fields) {
        data << field->text().trimmed();
    }
    return {data[0], data[1], data[2], data[3]};
}

bool MainWindow::validateInput(const QStringList &fields)
{
    return std::all_of(fields.begin(), fields.end(), [](const QString &field) {
        return !field.isEmpty();
    });
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


void MainWindow::setupStockManager()
{
    m_stockManager = new StockManager(m_dbHandler, this);

    // Connect signals
    connect(m_stockManager, &StockManager::stockUpdated, this, &MainWindow::onStockUpdated);

    // Set headers for stock table
    QStringList headers = {"Product ID", "Product Name", "Price/Unit", "Category","Total Quantity", "Remaining Quantity"};
    ui->stockTable->setColumnCount(headers.size());
    ui->stockTable->setHorizontalHeaderLabels(headers);
    ui->workerStockTable->setColumnCount(headers.size());
    ui->workerStockTable->setHorizontalHeaderLabels(headers);

    // Configure table properties
    ui->stockTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->stockTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->stockTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->stockTable->verticalHeader()->setVisible(false);

    ui->workerStockTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->workerStockTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->workerStockTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->workerStockTable->verticalHeader()->setVisible(false);

    // Load initial data
    refreshStockTable();
}

void MainWindow::refreshStockTable()
{
    if (m_dbHandler->isConnected()) {
        m_stockManager->loadStock(ui->stockTable);
        m_stockManager->loadStock(ui->workerStockTable);
    }
}

void MainWindow::onStockUpdated()
{
    refreshStockTable();
    updateDashboard();
}

void MainWindow::on_viewStockSearchEdit_textChanged(const QString &arg1)
{
    if (arg1.isEmpty()) {
        refreshStockTable();
    } else {
        m_stockManager->searchStock(ui->stockTable, arg1);
    }
}
void MainWindow::on_workerStockSearchEdit_textChanged(const QString &arg1)
{
    if (arg1.isEmpty()) {
        refreshStockTable();
    } else {
        m_stockManager->searchStock(ui->stockTable, arg1);
    }
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
    setupSalesTable(ui->searchProductTable_2, {"ID", "Name", "Price", "Category", "Stock", "Updated"}, 6);
    setupSalesTable(ui->selectedProductsTable_2, {"Product", "Price", "Qty", "Total", "Remove"}, 5);
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
    connect(ui->searchProductTable_2, &QTableWidget::cellClicked,
            this, &MainWindow::on_searchProductTable_cellClicked);
    connect(ui->selectedProductsTable_2, &QTableWidget::cellClicked,
            this, &MainWindow::on_selectedProductsTable_cellClicked);

    // Quantity controls

    // Action buttons
    connect(ui->sellProductsBtn_2, &QPushButton::clicked,
            this, &MainWindow::on_sellProductsBtn_clicked);
    connect(ui->clearSelectionBtn_2, &QPushButton::clicked,
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
    if (m_productManager && ui->searchProductTable_2) {
        m_productManager->loadProducts(ui->searchProductTable_2);
    }
}

void MainWindow::refreshSelectedProductsTable()
{
    if (!ui->selectedProductsTable_2) return;

    ui->selectedProductsTable_2->setRowCount(0);

    for (int i = 0; i < m_selectedItems.size(); i++) {
        const SaleItem &item = m_selectedItems[i];
        int row = ui->selectedProductsTable_2->rowCount();
        ui->selectedProductsTable_2->insertRow(row);

        // Set item data
        ui->selectedProductsTable_2->setItem(row, 0, new QTableWidgetItem(item.productName));
        ui->selectedProductsTable_2->setItem(row, 1, new QTableWidgetItem(QString::number(item.unitPrice, 'f', 2)));
        ui->selectedProductsTable_2->setItem(row, 2, new QTableWidgetItem(QString::number(item.quantity)));
        ui->selectedProductsTable_2->setItem(row, 3, new QTableWidgetItem(QString::number(item.totalPrice, 'f', 2)));

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

        ui->selectedProductsTable_2->setCellWidget(row, 4, removeBtn);
    }

    updateSalesTotals();
}

void MainWindow::updateSalesTotals()
{
    m_totalAmount = 0.0;
    for (const SaleItem &item : m_selectedItems) {
        m_totalAmount += item.totalPrice;
    }

    if (ui->labelTotalPrice_2) {
        ui->labelTotalPrice_2->setText(QString("%1 Rs.").arg(m_totalAmount, 0, 'f', 2));
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
    if (!ui->selectedProductsTable_2) return;

    // Clear all highlights first
    for (int i = 0; i < ui->selectedProductsTable_2->rowCount(); i++) {
        for (int j = 0; j < ui->selectedProductsTable_2->columnCount(); j++) {
            auto *item = ui->selectedProductsTable_2->item(i, j);
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
    if (!m_productManager || !ui->searchProductTable_2) return;

    if (text.isEmpty()) {
        refreshProductSalesTable();
    } else {
        m_productManager->searchProducts(ui->searchProductTable_2, text);
    }
}

void MainWindow::on_salesSearchEdit_textChanged(const QString &text)
{
    if (!m_salesManager || !ui->salesTable) return;

    m_salesManager->searchSales(ui->salesTable, text);
}

void MainWindow::on_searchProductTable_cellClicked(int row, int column)
{
    if (!ui->searchProductTable_2 || row < 0 || row >= ui->searchProductTable_2->rowCount()) return;

    // Extract product data from table
    SaleItem item;
    item.productId = ui->searchProductTable_2->item(row, 0)->text().toInt();
    item.productName = ui->searchProductTable_2->item(row, 1)->text();
    item.unitPrice = ui->searchProductTable_2->item(row, 2)->text().toDouble();
    item.category = ui->searchProductTable_2->item(row, 3)->text();
    item.available = ui->searchProductTable_2->item(row, 4)->text().toInt();
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

void MainWindow::onSalesUpdated()
{
    refreshSalesTable();
    refreshProductSalesTable(); // Refresh products to show updated stock
    updateDashboard(); // Update main dashboard with new sales data

    // Update sales dashboard if it exists
    if (m_salesdashboard) {
        m_salesdashboard->refreshData();
    }
}

void MainWindow::updateDashboard()
{
    // Update debtor statistics on the dashboard
    int totalDebtors = 0;
    double totalDebt = 0.0;
    int totalProducts = 0;
    int totalStock = 0;
    int totalSales = 0;
    double totalAmount = 0.0;
    double profitMargin = 0.0;

    if (m_salesManager->getSalesStats(totalSales, totalAmount, profitMargin)) {
        ui->labelTotalSales->setText(QString::number(totalSales));
        ui->labelTotalAmount->setText(QString::number(totalAmount));

    }

    if (m_debtManager->getDebtorStats(totalDebtors, totalDebt)) {
        // Update dashboard labels with the obtained stats
        ui->labelTotalDebtors->setText(QString::number(totalDebtors));
        ui->labelTotalDebt_2->setText(QString("$%1").arg(totalDebt, 0, 'f', 2));
    }
    if (m_productManager->getProductStats(totalProducts, totalStock)) {
        ui->labelTotalProducts->setText(QString::number(totalProducts));
    }
    // Update other dashboard stats as needed...

}



void MainWindow::on_cross_2_clicked()
{
    if (m_dbHandler->isAdmin()==true){
        ui->stackedWidget->setCurrentIndex(3);
    }
    else{
        ui->stackedWidget->setCurrentIndex(9);
    }
}

