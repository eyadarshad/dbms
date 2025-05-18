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
    , m_authManager(new AuthManager(m_dbHandler, this))
    , m_debtManager(new DebtManager(m_dbHandler, this))
    , m_productManager(new ProductManager(m_dbHandler, this))
    , m_vendorManager(new VendorManager(m_dbHandler, this))
    , m_workManager(new WorkerManager(m_dbHandler, this))
    , m_salesManager(new SalesManager(m_dbHandler, this))
{
    ui->setupUi(this);
    isDarkMode = true;
    // m_vendorManager = new VendorManager(m_dbHandler, this);
    // m_workManager = new WorkerManager(m_dbHandler, this);
    // m_stockManager = new StockManager(m_dbHandler, this);
    // Initialize SalesManager
    m_salesManager = new SalesManager(m_dbHandler, this);


    // Setup core functionality
    setupIconManagement();
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
        ui->pushButton_55, ui->pushButton_68, ui->pushButton_43, ui->pushButton_28,
        ui->pushButton_44, ui->pushButton_93, ui->pushButton_109
    };

    QList<QPushButton*> darkModeButtons = {
        ui->pushButton_84, ui->pushButton_48, ui->pushButton_74, ui->pushButton_66,
        ui->pushButton_56, ui->pushButton_67, ui->pushButton_106, ui->pushButton_99,
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

    // Initialize managers
    m_authManager = new AuthManager(m_dbHandler, this);
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
    // setupStockManager();
    setupSalesManager();


    // Initialize sales-related variables
    m_totalAmount = 0.0;
    m_selectedItems.clear();

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
    ui->gridLayout->addWidget(ui->display, 0, 0, 1, 3);

    // Add backspace button
    QPushButton *backspaceBtn = new QPushButton("⌫");
    backspaceBtn->setFixedSize(60, 60);
    ui->gridLayout->addWidget(backspaceBtn, 0, 3);
    connect(backspaceBtn, &QPushButton::clicked, this, [=]() {
        QString text = ui->display->text();
        if (!text.isEmpty())
            ui->display->setText(text.left(text.length() - 1));
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
            ui->gridLayout->addWidget(btn, row, col);

            connect(btn, &QPushButton::clicked, this, [=]() {
                QString val = btn->text();
                if (val == "=") {
                    QJSEngine engine;
                    QJSValue result = engine.evaluate(ui->display->text());
                    ui->display->setText(result.toString());
                } else if (val == "C") {
                    ui->display->clear();
                } else {
                    ui->display->setText(ui->display->text() + val);
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
        {ui->pagew34, 11}, {ui->pages1, 8}, {ui->pages2, 9}, {ui->pages3, 10},
        {ui->pages4, 11}, {ui->btnlogout, 0}, {ui->btnlogout_2, 0},
        {ui->btnlogout_3, 0}, {ui->btnlogout_4, 0}, {ui->btnlogout_5, 0},
        {ui->btnlogout_6, 0}, {ui->btnlogout_7, 0}, {ui->btnlogout_8, 0},
        {ui->btnlogout_9, 0}, {ui->btnlogout_10, 0}, {ui->btnlogout_11, 0},
        {ui->btnlogout_12, 0}, {ui->cross, 7}, {ui->cross_2, 3},
        {ui->addProductBtn, 14}, {ui->addWorkerBtn, 12}, {ui->addDebtorBtn, 13}, {ui->cross_3, 2},
        {ui->addProductBtn_2, 14},{ui->addProductBtn_4, 3},{ui->addVendorBtn_2, 5},{ui->addVendorBtn, 15}, {ui->addWorkerBtn_2,7}, {ui->addDebtorBtn_2, 2}
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
        "QMessageBox { background-color: #2b2b2b; color: white; }"
        "QPushButton { background-color: #444; color: white; padding: 5px; width:8px; border-radius: 5px; }"
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

void MainWindow::setupIconManagement() {
    // Define mapping patterns for dark/light mode icon pairs
    iconPairMappings = {
        {"dashboard-square-02-Stroke-Rounded (1).png", "dashboard-square-02-Stroke-Rounded.png"},
        {"folder-view-Stroke-Rounded (1).png", "folder-view-Stroke-Rounded.png"},
        {"package-Stroke-Rounded (1).png", "package-Stroke-Rounded.png"},
        {"invoice-03-Stroke-Rounded.png", "invoice-Stroke-Rounded.png"},
        {"user-circle-Stroke-Rounded.png", "user-circle-02-Stroke-Rounded.png"},
        {"shopping-cart-02-Stroke-Rounded (1).png", "shopping-cart-02-Stroke-Rounded.png"},
        {"invoice-02-Stroke-Rounded (1).png", "invoice-02-Stroke-Rounded.png"},
        {"cancel-01-Stroke-Rounded.png", "cancel-01-Stroke-Rounded (1).png"},
        {"locked-Stroke-Rounded.png", "locked-Stroke-Rounded (2).png"},
        {"logout-05-Stroke-Rounded.png", "logout-05-Stroke-Rounded (1).png"},
        {"eyebrow (1).png", "eyebrow.png"},
        {"view-Stroke-Rounded.png", "view-Stroke-Rounded (1).png"},
        {"search-01-Stroke-Rounded (1).png", "search-01-Stroke-Rounded.png"},
        {"home-02-Stroke-Rounded.png", "home-Stroke-Rounded.png"},
        {"notifications-Stroke-Rounded.png", "notifications-02-Stroke-Rounded.png"},
        {"settings-02-Stroke-Rounded.png", "settings-Stroke-Rounded.png"},
        {"help-02-Stroke-Rounded.png", "help-Stroke-Rounded.png"},
        {"add-user-Stroke-Rounded.png", "add-user-02-Stroke-Rounded.png"},
        {"chat-02-Stroke-Rounded.png", "chat-Stroke-Rounded.png"},
        {"support-Stroke-Rounded.png", "support-02-Stroke-Rounded.png"}
    };
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

    if (m_authManager->login(username, password)) {
        // Login successful - now check if admin or worker
        if (m_authManager->isAdmin()) {
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
    m_authManager->logout();
    ui->stackedWidget->setCurrentIndex(0); // Back to login page
    ui->username_login->clear();
    ui->password_login->clear();
}

void MainWindow::setupDebtManager()
{
    m_debtManager = new DebtManager(m_dbHandler, this);

    // Connect signals
    connect(m_debtManager, &DebtManager::debtorsUpdated, this, &MainWindow::onDebtorsUpdated);

    // Set headers for debtors table
    QStringList headers = {"ID", "Name", "Contact", "Email", "Address", "Amount", "Date"};
    ui->debtorsTable->setColumnCount(headers.size());
    ui->debtorsTable->setHorizontalHeaderLabels(headers);

    // Load data immediately
    refreshDebtorTable();
}

// Add this helper function
void MainWindow::refreshDebtorTable()
{
    if (m_dbHandler->isConnected()) {
        m_debtManager->loadDebtors(ui->debtorsTable);
    }
}

void MainWindow::on_addDebtorBtn_clicked()
{
    // Navigate to the debtor form page (stackedWidget index 13)
    ui->stackedWidget->setCurrentIndex(13);

    // Clear the form fields
    ui->debtorNameEdit->clear();
    ui->debtorContactEdit->clear();
    ui->debtorAddressEdit->clear();
    ui->debtorAmountEdit->clear();
    ui->dateEdit->setDate(QDate::currentDate());
}

void MainWindow::on_addDebtorBtn_2_clicked()
{
    // Get form data
    QString name = ui->debtorNameEdit->text().trimmed();
    QString contact = ui->debtorContactEdit->text().trimmed();
    QString email = ""; // You may want to add this field to your UI
    QString address = ui->debtorAddressEdit->text().trimmed();
    double debtAmount = ui->debtorAmountEdit->text().toDouble();
    QDate dateIncurred = ui->dateEdit->date();

    // Validate form data
    if (name.isEmpty() || contact.isEmpty() || address.isEmpty() || debtAmount <= 0) {
        QMessageBox::warning(this, "Missing Information", "Please fill in all required fields.");
        return;
    }

    // Add the debtor to the database
    if (m_debtManager->addDebtor(name, contact, email, address, debtAmount, dateIncurred)) {
        QMessageBox::information(this, "Success", "Debtor added successfully.");

        // Return to the debtor management page (index 2)
        ui->stackedWidget->setCurrentIndex(2);

        // Refresh the table to show the new debtor
        refreshDebtorTable();
    } else {
        QMessageBox::critical(this, "Error", "Failed to add debtor. Please try again.");
    }
}

void MainWindow::on_removeDebtorBtn_clicked()
{
    // Get the currently selected row
    int currentRow = ui->debtorsTable->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "No Selection", "Please select a debtor to remove.");
        return;
    }

    // Get the debtor ID from the hidden column
    int debtorId = ui->debtorsTable->item(currentRow, 0)->text().toInt();
    QString debtorName = ui->debtorsTable->item(currentRow, 1)->text();

    // Confirm deletion
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirm Removal",
                                  "Are you sure you want to remove debtor: " + debtorName + "?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (m_debtManager->removeDebtor(debtorId)) {
            QMessageBox::information(this, "Success", "Debtor removed successfully.");
            refreshDebtorTable();
        } else {
            QMessageBox::critical(this, "Error", "Failed to remove debtor.");
        }
    }
}

void MainWindow::on_debtorSearchEdit_textChanged(const QString &arg1)
{
    // If connected to database and text is not empty, search
    if (m_dbHandler->isConnected()) {
        if (arg1.isEmpty()) {
            refreshDebtorTable();
        } else {
            m_debtManager->searchDebtors(ui->debtorsTable, arg1);
        }
    }
}

void MainWindow::onDebtorsUpdated()
{
    // Refresh the table when debtors are updated
    refreshDebtorTable();
    updateDashboard();
}



// You need to add this function to your existing updateDashboard method
// or modify it to include these updates

void MainWindow::setupProductManager()
{
    m_productManager = new ProductManager(m_dbHandler, this);

    // Connect signals and slots
    connect(m_productManager, &ProductManager::productsUpdated, this, &MainWindow::onProductsUpdated);

    // Set headers for products table
    QStringList headers = {"ID", "Name", "Price", "Category", "Quantity", "Added At"};
    ui->productsTable->setColumnCount(headers.size());
    ui->productsTable->setHorizontalHeaderLabels(headers);

    // Load products data
    refreshProductTable();

    // Set up category combo box with common categories
    ui->categoryCombo->clear();
    ui->categoryCombo->addItem("Electronics");
    ui->categoryCombo->addItem("Clothing");
    ui->categoryCombo->addItem("Food");
    ui->categoryCombo->addItem("Beverages");
    ui->categoryCombo->addItem("Household");
    ui->categoryCombo->addItem("Other");
}

void MainWindow::refreshProductTable()
{
    // Load products data into table
    if (m_dbHandler->isConnected()) {
        m_productManager->loadProducts(ui->productsTable);
    }
}
void MainWindow::on_addProductBtn_clicked()
{
    // Clear the form fields
    ui->productNameEdit->clear();
    ui->priceEdit->clear();
    ui->categoryCombo->setCurrentIndex(0);
    ui->quantityEdit->clear();
    ui->dateEdit_2->setDate(QDate::currentDate());

    // Navigate to the add product form page
    ui->stackedWidget->setCurrentIndex(14);  // Adjust this index if needed
}

void MainWindow::on_addProductBtn_2_clicked()
{
    // Get data from form fields
    QString name = ui->productNameEdit->text();
    double price = ui->priceEdit->text().toDouble();
    QString category = ui->categoryCombo->currentText();
    int quantity = ui->quantityEdit->text().toInt();
    QDate date = ui->dateEdit_2->date();

    // Validate inputs
    if (name.isEmpty() || price <= 0 || quantity < 0) {
        showDarkMessageBox("Invalid Input", "Please enter valid product details.");
        return;
    }

    // Add product
    if (m_productManager->addProduct(name, price, category, quantity, date)) {
        // Clear form fields
        ui->productNameEdit->clear();
        ui->priceEdit->clear();
        ui->quantityEdit->clear();
        ui->dateEdit_2->setDate(QDate::currentDate());

        // Create message box
        QMessageBox msgBox;
        msgBox.setText("Information saved successfully");
        msgBox.setWindowTitle("Success");
        msgBox.setStandardButtons(QMessageBox::Ok);

        // Show message box and handle response
        if (msgBox.exec() == QMessageBox::Ok) {
            // Switch to index 3 when OK is clicked
            ui->stackedWidget->setCurrentIndex(3);
        }
    } else {
        showDarkMessageBox("Error", "Failed to add product.");
    }
}

void MainWindow::on_removeProductBtn_clicked()
{
    // Check if a row is selected
    int selectedRow = ui->productsTable->currentRow();
    if (selectedRow < 0) {
        showDarkMessageBox("Error", "Please select a product to remove!");
        return;
    }

    // Get the product ID from the first column
    int productId = ui->productsTable->item(selectedRow, 0)->text().toInt();
    QString productName = ui->productsTable->item(selectedRow, 1)->text();

     if (m_productManager->removeProduct(productId)) {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirm Removal",
                                  "Are you sure you want to remove debtor: " + productName + "?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (m_debtManager->removeDebtor(productId)) {
            QMessageBox::information(this, "Success", "Product removed successfully.");
            refreshDebtorTable();
        }
    }
    // Remove the product
}
     else {
        showDarkMessageBox("Error", "Failed to remove product!");
    }
refreshProductTable();
}

void MainWindow::onProductsUpdated()
{
    // Refresh the product table
    refreshProductTable();

    // Update dashboard statistics
    updateDashboard();
}

// Add search functionality
void MainWindow::on_productSearchEdit_textChanged(const QString &arg1)
{
    // If search text is empty, show all products
    if (arg1.isEmpty()) {
        refreshProductTable();
    } else {
        // Otherwise, search for products matching the text
        m_productManager->searchProducts(ui->productsTable, arg1);
    }
}
// Add these improved implementations to your MainWindow class

void MainWindow::setupVendorManager()
{
    m_vendorManager = new VendorManager(m_dbHandler, this);

    // Connect signals and slots
    connect(m_vendorManager, &VendorManager::vendorsUpdated, this, &MainWindow::onVendorsUpdated);

    // Set headers for vendors table
    QStringList headers = {"ID", "Name", "Contact", "Address", "Payment", "Date of Supply"};
    ui->vendorsTable->setColumnCount(headers.size());
    ui->vendorsTable->setHorizontalHeaderLabels(headers);

    // Load vendors data
    refreshVendorTable();
}

void MainWindow::setupWorkerManager()
{
    m_workManager = new WorkerManager(m_dbHandler, this);

    // Connect signals and slots
    connect(m_workManager, &WorkerManager::workersUpdated, this, &MainWindow::onWorkersUpdated);

    // Set headers for workers table
    QStringList headers = {"ID", "Name", "Contact", "Email", "Status", "Salary", "Date of Joining"};
    ui->workersTable->setColumnCount(headers.size());
    ui->workersTable->setHorizontalHeaderLabels(headers);

    // Load workers data
    refreshWorkerTable();

    // Set up status combo box with common statuses
    ui->statusCombo->clear();
    ui->statusCombo->addItem("Active");
    ui->statusCombo->addItem("On Leave");
    ui->statusCombo->addItem("Terminated");
    ui->statusCombo->addItem("Suspended");
    ui->statusCombo->addItem("Part-time");
}

// Update the refreshVendorTable and refreshWorkerTable methods for consistency
void MainWindow::refreshVendorTable()
{
    // Load vendors data into table
    if (m_dbHandler->isConnected() && m_vendorManager) {
        m_vendorManager->loadVendors(ui->vendorsTable);
    }
}

void MainWindow::refreshWorkerTable()
{
    // Load workers data into table
    if (m_dbHandler->isConnected() && m_workManager) {
        m_workManager->loadWorkers(ui->workersTable);
    }
}

// Update the onVendorsUpdated and onWorkersUpdated methods to also update the dashboard
void MainWindow::onVendorsUpdated()
{
    // Refresh the vendor table
    refreshVendorTable();

    // Update dashboard statistics
    updateDashboard();
}

void MainWindow::onWorkersUpdated()
{
    // Refresh the worker table
    refreshWorkerTable();

    // Update dashboard statistics
    updateDashboard();
}

// Vendor management slots
void MainWindow::on_addVendorBtn_clicked()
{
    ui->vendorNameEdit->clear();
    ui->vendorContactEdit->clear();
    ui->vendorAddressEdit->clear();
    ui->vendorPaymentEdit->clear();
    // Switch to vendor form (index 15)
    ui->stackedWidget->setCurrentIndex(15);
}

void MainWindow::on_addVendorBtn_2_clicked()
{
    // Get data from form fields
    QString name = ui->vendorNameEdit->text().trimmed();
    QString contact = ui->vendorContactEdit->text().trimmed();
    QString address = ui->vendorAddressEdit->text().trimmed();
    QString cashStr = ui->vendorPaymentEdit->text().trimmed();
    double cash = cashStr.toDouble();
    QDate dateOfSupply = ui->dateEdit_4->date();

    // Validate inputs
    if (name.isEmpty() || contact.isEmpty() || address.isEmpty() || cashStr.isEmpty()) {
        showDarkMessageBox("Error", "All fields are required!");
        return;
    }

    // Add vendor to database
    if (m_vendorManager->addVendor(name, address, contact, cash, dateOfSupply)) {
        // Clear form fields
        ui->vendorNameEdit->clear();
        ui->vendorContactEdit->clear();
        ui->vendorAddressEdit->clear();
        ui->vendorPaymentEdit->clear();

        // Return to vendor management page (index 4)
        ui->stackedWidget->setCurrentIndex(4);

        showDarkMessageBox("Success", "Vendor added successfully!");
    } else {
        showDarkMessageBox("Error", "Failed to add vendor!");
    }
}

void MainWindow::on_removeVendorBtn_clicked()
{
    // Check if a row is selected
    QTableWidget* table = ui->vendorsTable;
    if (!table || table->selectedItems().isEmpty()) {
        showDarkMessageBox("Error", "Please select a vendor to remove!");
        return;
    }

    // Get vendor ID from the selected row
    int row = table->selectedItems().first()->row();
    bool ok;
    int vendorId = table->item(row, 0)->text().toInt(&ok);

    if (!ok) {
        showDarkMessageBox("Error", "Invalid vendor ID!");
        return;
    }

    // Remove vendor from database
    if (m_vendorManager->removeVendor(vendorId)) {
        showDarkMessageBox("Success", "Vendor removed successfully!");
    } else {
        showDarkMessageBox("Error", "Failed to remove vendor!");
    }
}

void MainWindow::on_vendorSearchEdit_textChanged(const QString &arg1)
{
    // Search vendors based on text input
    if (m_vendorManager) {
        m_vendorManager->searchVendors(ui->vendorsTable, arg1);
    }
}

// Worker management slots
void MainWindow::on_addWorkerBtn_clicked()
{
    // Clear the worker form fields
    ui->workerNameEdit->clear();
    ui->workerContactEdit->clear();
    ui->workerEmailEdit->clear();
    ui->workerSalaryEdit->clear();
    ui->statusCombo->setCurrentIndex(0);
    ui->dateEdit_3->setDate(QDate::currentDate());

    // Navigate to the add worker form page
    ui->stackedWidget->setCurrentIndex(12);  // Adjust index if needed
}

void MainWindow::on_addWorkerBtn_2_clicked()
{
    // Get data from form fields
    QString name = ui->workerNameEdit->text().trimmed();
    QString contact = ui->workerContactEdit->text().trimmed();
    QString email = ui->workerEmailEdit->text().trimmed(); // Assuming there's no email field in your form
    QString status = ui->statusCombo->currentText();
    QString salaryStr = ui->workerSalaryEdit->text().trimmed();
    double salary = salaryStr.toDouble();
    QDate dateOfJoining = ui->dateEdit_3->date();

    // Validate inputs
    if (name.isEmpty() || contact.isEmpty() || salaryStr.isEmpty()) {
        showDarkMessageBox("Error", "All fields are required!");
        return;
    }

    // Add worker to database
    if (m_workManager->addWorker(name, contact, email, status, salary, dateOfJoining)) {
        // Clear form fields
        ui->workerNameEdit->clear();
        ui->workerContactEdit->clear();
        ui->workerSalaryEdit->clear();

        // Return to worker management page (index 7)
        ui->stackedWidget->setCurrentIndex(7);

        showDarkMessageBox("Success", "Worker added successfully!");
    } else {
        showDarkMessageBox("Error", "Failed to add worker!");
    }
}

void MainWindow::on_removeWorkerBtn_clicked()
{
    // Check if a row is selected
    QTableWidget* table = ui->workersTable;
    if (!table || table->selectedItems().isEmpty()) {
        showDarkMessageBox("Error", "Please select a worker to remove!");
        return;
    }

    // Get worker ID from the selected row
    int row = table->selectedItems().first()->row();
    bool ok;
    int workerId = table->item(row, 0)->text().toInt(&ok);

    if (!ok) {
        showDarkMessageBox("Error", "Invalid worker ID!");
        return;
    }

    // Remove worker from database
    if (m_workManager->removeWorker(workerId)) {
        showDarkMessageBox("Success", "Worker removed successfully!");
    } else {
        showDarkMessageBox("Error", "Failed to remove worker!");
    }
}

void MainWindow::on_workerSearchEdit_textChanged(const QString &arg1)
{
    // Search workers based on text input
    if (m_workManager) {
        m_workManager->searchWorkers(ui->workersTable, arg1);
    }
}

void MainWindow::setupSalesManager()
{
    // Verify that the sales tab exists in your UI
    if (!ui->salesTab) {
        qDebug() << "Error: salesTab widget not found in UI!";
        return;
    }

    // Create SalesDashboard instance
    SalesDashboard* salesDashboard = new SalesDashboard(
        m_dbHandler,
        m_productManager,
        m_salesManager,
        m_authManager,
        ui->salesTab);

    // Setup layout for the sales tab
    QVBoxLayout* salesLayout = new QVBoxLayout(ui->salesTab);
    salesLayout->setContentsMargins(0, 0, 0, 0);
    salesLayout->addWidget(salesDashboard);
    ui->salesTab->setLayout(salesLayout);

    // Connect signals from sales manager to our handler
    connect(m_salesManager, &SalesManager::salesUpdated, this, &MainWindow::onSalesUpdated);

    // If you want, you can style the sales dashboard to match your theme
    if (isDarkMode) {
        // Apply dark mode styles if needed
        salesDashboard->setStyleSheet(originalMainStylesheet);
    } else {
        // Apply light mode styles if needed
        QString lightModeStyles = processStyleSheetForLightMode(originalMainStylesheet);
        salesDashboard->setStyleSheet(lightModeStyles);
    }
}
void MainWindow::onSalesUpdated()
{
    // Refresh sales-related UI elements
    refreshSalesTable();
    refreshProductSalesTable();

    // Update dashboard stats
    updateDashboard();
}

void MainWindow::refreshSalesTable()
{
    // This would be handled by the SalesDashboard component now
    // If you have a separate sales table in the main window, use:
    m_salesManager->loadSales(ui->salesTable);
}

void MainWindow::refreshProductSalesTable()
{
    // This would be handled by the SalesDashboard component
    // If you have a separate product table for sales in the main window, use:
    // m_productManager->loadProducts(ui->productSalesTableWidget);
}

void MainWindow::refreshSelectedProductsTable()
{
    // Implementation depends on how you store selected products
    // This is now handled in SalesDashboard
}

void MainWindow::updateSalesTotals()
{
    // Calculate the total amount from selected items
    m_totalAmount = 0.0;

    for (const SaleItem &item : m_selectedItems) {
        m_totalAmount += item.totalPrice;
    }
    updateDashboard();

    // Update UI element with total
    // Example: ui->totalAmountLabel->setText(QString("Total: %1 Rs.").arg(m_totalAmount, 0, 'f', 2));
}
void MainWindow::on_productSalesSearchEdit_textChanged(const QString &arg1)
{
    // Forward to sales dashboard or handle directly if needed
    // The SalesDashboard should handle this internally
}

void MainWindow::on_salesSearchEdit_textChanged(const QString &arg1)
{
    // Forward to sales dashboard or handle directly if needed
    // The SalesDashboard should handle this internally
}

void MainWindow::on_addQtyBtn_clicked()
{
    // Forward to sales dashboard or handle directly if needed
    // The SalesDashboard should handle this internally
}

void MainWindow::on_removeQtyBtn_clicked()
{
    // Forward to sales dashboard or handle directly if needed
    // The SalesDashboard should handle this internally
}

void MainWindow::on_sellProductsBtn_clicked()
{
    // Forward to sales dashboard or handle directly if needed
    // The SalesDashboard should handle this internally
}

void MainWindow::on_clearSelectionBtn_clicked()
{
    // Forward to sales dashboard or handle directly if needed
    // The SalesDashboard should handle this internally
}

void MainWindow::on_productsTable_cellClicked(int row, int column)
{
    // Forward to sales dashboard or handle directly if needed
    // The SalesDashboard should handle this internally
    Q_UNUSED(row);
    Q_UNUSED(column);
}

void MainWindow::on_selectedProductsTable_cellClicked(int row, int column)
{
    // Forward to sales dashboard or handle directly if needed
    // The SalesDashboard should handle this internally
    Q_UNUSED(row);
    Q_UNUSED(column);
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

    if (m_salesManager && m_salesManager->getSalesStats(totalSales, totalAmount, profitMargin)) {
        // Update UI elements with sales stats - replace with your actual UI element names
        ui->salesCountLabel->setText(QString::number(totalSales));
        ui->profitMarginLabel_3->setText(QString::number(profitMargin, 'f', 1) + "%");
    }

    if (m_productManager && m_productManager->getProductStats(totalProducts, totalStock)) {
        // Update product stats on dashboard - replace with your actual UI element names
        ui->productCountLabel->setText(QString::number(totalProducts));
    }

    if (m_debtManager->getDebtorStats(totalDebtors, totalDebt)) {
        // Update dashboard labels with the obtained stats
        ui->labelTotalDebtors->setText(QString::number(totalDebtors));
        ui->labelTotalDebt->setText(QString("$%1").arg(totalDebt, 0, 'f', 2));
    }
    if (m_productManager->getProductStats(totalProducts, totalStock)) {
        ui->productCountLabel->setText(QString::number(totalProducts));
    }
    // Update other dashboard stats as needed...
}



// The slot implementations for sales actions are now handled by SalesDashboard
// We keep these in MainWindow for compatibility with Qt's signal-slot connections
// But they can be minimal since most logic is in SalesDashboard
