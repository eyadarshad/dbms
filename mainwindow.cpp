#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>
#include <QColor>
#include <QRegularExpression>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    isDarkMode = true;

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

    // Setup database connection
    DBConnection = QSqlDatabase::addDatabase("QMYSQL");
    DBConnection.setHostName("localhost");
    DBConnection.setDatabaseName("eyad");
    DBConnection.setUserName("root");
    DBConnection.setPassword("");

    if (!DBConnection.open()) {
        showDarkMessageBox("Database Connection",
                           "Failed to connect to database: " + DBConnection.lastError().text());
    }
}

MainWindow::~MainWindow() {
    delete ui;
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
        {ui->addprod, 13}, {ui->addworker111, 12}, {ui->pushButton_76, 13}, {ui->cross_3, 2}
    };

    for (auto it = navMap.begin(); it != navMap.end(); ++it) {
        connectPageButton(it.key(), it.value());
    }
}

void MainWindow::on_pushButton_clicked() {
    // Empty implementation as placeholder
}

void MainWindow::on_pushButton_95_clicked() {
    QString file_name = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                     QDir::homePath(), tr("Images (*.png *.xpm *.jpg)"));
    if (file_name.isEmpty()) return;

    // Display image
    QImage img(file_name);
    QPixmap pix = QPixmap::fromImage(img);
    int w = ui->label_pic->width();
    int h = ui->label_pic->height();
    ui->label_pic->setPixmap(pix.scaled(w, h, Qt::IgnoreAspectRatio));

    // Process image for black pixels
    unsigned int cols = img.width();
    unsigned int rows = img.height();
    unsigned int numBlackPixels = 0;
    QVector<QVector<int>> imgArray(rows, QVector<int>(cols, 0));

    // Extract pixel data
    for (unsigned int i = 0; i < rows; i++) {
        for (unsigned int j = 0; j < cols; j++) {
            QColor clrCurrent(img.pixel(j, i));
            int r = clrCurrent.red();
            int g = clrCurrent.green();
            int b = clrCurrent.blue();
            int a = clrCurrent.alpha();
            if (r + g + b < 20 && a > 240) {
                imgArray[i][j] = 1;
                numBlackPixels += 1;
            }
        }
    }

    // Save results to file
    QString filename = "C:/Users/EYAD/Download/text.txt";
    QFile fileout(filename);
    if (fileout.open(QFile::ReadWrite | QFile::Text)) {
        QTextStream out(&fileout);
        for (unsigned int i = 0; i < rows; i++) {
            for (unsigned int j = 0; j < cols; j++) {
                out << imgArray[i][j];
            }
            out << " " << Qt::endl;
        }
    }
}

void MainWindow::on_loginbtn_clicked() {
    QString Username = ui->username_login->text();
    QString Password = ui->password_login->text();

    if (Username == "Eyad" && Password == "jbh") {
        ui->stackedWidget->setCurrentIndex(1);
        return;
    }

    QSqlQuery Query(DBConnection);
    Query.prepare(
        "SELECT 'login_master' FROM login_master WHERE name = :username AND password = :password "
        "UNION "
        "SELECT 'login_worker' FROM login_worker WHERE name_w = :username AND password_w = :password"
        );
    Query.bindValue(":username", Username);
    Query.bindValue(":password", Password);

    if (Query.exec() && Query.next()) {
        QString tableName = Query.value(0).toString();
        ui->stackedWidget->setCurrentIndex(tableName == "login_master" ? 1 : 8);
    } else {
        showDarkMessageBox("Login Failed", "Incorrect Username or Password!");
    }
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

