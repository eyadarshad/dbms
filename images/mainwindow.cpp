#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QPushButton>
#include <QJSEngine>
#include <QJSValue>
#include <QMap>
#include <QShowEvent>
#include <QTimer>
#include <QFile>
#include <QApplication>
#include <QPalette>
#include <QColor>
#include <QIcon>
#include <QGraphicsEffect>
#include <QPixmap>
#include <QString>
#include <QList>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow){

    ui->setupUi(this);
    // Initially set to dark mode
    isDarkMode = true;

    // Connect light/dark mode buttons
    connect(ui->pushButton_83, &QPushButton::clicked, this, &MainWindow::enableLightMode);
    connect(ui->pushButton_84, &QPushButton::clicked, this, &MainWindow::enableDarkMode);

    // Store the original sidebar stylesheet for preservation
    sidebarStyleSheet = ui->frame_8->styleSheet();




    ui->eyeButton->setIcon(QIcon(":/new/prefix1/images/view-Stroke-Rounded.png"));
    ui->password_login->setEchoMode(QLineEdit::Password);
    DBConnection = QSqlDatabase::addDatabase("QMYSQL");
    DBConnection.setHostName("localhost");   // or IP address of your database server
    DBConnection.setDatabaseName("eyad"); // database name you created
    DBConnection.setUserName("root");  // usually 'root' or any user you made
    DBConnection.setPassword("");

    if (!DBConnection.open()) {
        QMessageBox::critical(this, "Database Connection", "Failed to connect to database: " + DBConnection.lastError().text());
    }

    // setWindowFlag(Qt::FramelessWindowHint);
    setupChart();
    setupCalculator();
    setupNavigation();

}


// Call this in your constructor after setupUi:


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


    QPushButton *backspaceBtn = new QPushButton("⌫");
    backspaceBtn->setFixedSize(60, 60);
    ui->gridLayout->addWidget(backspaceBtn, 0, 3);
    connect(backspaceBtn, &QPushButton::clicked, this, [=]() {
        QString text = ui->display->text();
        if (!text.isEmpty())
            ui->display->setText(text.left(text.length() - 1));
    });

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
                                       {ui->addprod, 13}, {ui->addworker111,12}, {ui->pushButton_76,13}, {ui->cross_3,2}   };

    for (auto it = navMap.begin(); it != navMap.end(); ++it) {
        connectPageButton(it.key(), it.value());
    }


}
void MainWindow::on_pushButton_clicked()
{
    // your code here (even if empty)
}

void MainWindow::on_pushButton_95_clicked()
{
    QString file_name = QFileDialog::getOpenFileName(this, tr("Open File"), QDir::homePath(), tr("Images (*.png *.xpm *.jpg)"));
    if (!file_name.isEmpty()) {
        // open prompt and display image
        QMessageBox::information(this, "...", file_name);
        QImage img(file_name);
        QPixmap pix = QPixmap::fromImage(img);
        int w = ui->label_pic->width();
        int h = ui->label_pic->height();
        ui->label_pic->setPixmap(pix.scaled(w, h, Qt::IgnoreAspectRatio)); // <--- Changed here!

        // get image width and height, create empty binary matrix
        unsigned int cols = img.width();
        unsigned int rows = img.height();
        unsigned int numBlackPixels = 0;
        QVector<QVector<int>> imgArray(rows, QVector<int>(cols, 0));

        // get pixel data and update matrix
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
}

void MainWindow::on_loginbtn_clicked()
{
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


void MainWindow::showDarkMessageBox(const QString &title, const QString &message)
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(title);
    msgBox.setText(message);
    msgBox.setIcon(QMessageBox::Warning);

    // Apply dark stylesheet
    msgBox.setStyleSheet(
        "QMessageBox { background-color: #2b2b2b; color: white; }"
        "QPushButton { background-color: #444; color: white; padding: 5px; width:8px; border-radius: 5px; }"
        "QPushButton:hover { background-color: #666; }"
        );

    msgBox.exec();
}

bool passwordVisible = false;

void MainWindow::on_eyeButton_clicked()
{
    passwordVisible = !passwordVisible;

    if (passwordVisible) {
        ui->password_login->setEchoMode(QLineEdit::Normal);   // <-- Show text
        ui->eyeButton->setIcon(QIcon(":/new/prefix1/images/eyebrow (1).png")); // <-- Open eye
    } else {
        ui->password_login->setEchoMode(QLineEdit::Password); // <-- Hide text
        ui->eyeButton->setIcon(QIcon(":/new/prefix1/images/view-Stroke-Rounded.png")); // <-- Closed eye
    }
}
MainWindow::~MainWindow(){
    delete ui;
}

QColor MainWindow::invertColor(const QColor &color)
{
    return QColor(255 - color.red(), 255 - color.green(), 255 - color.blue(), color.alpha());
}

// Helper function to convert color string to QColor
QColor MainWindow::stringToColor(const QString &colorStr)
{
    if (colorStr.startsWith("#")) {
        // Handle hex colors
        return QColor(colorStr);
    } else if (colorStr.startsWith("rgb")) {
        // Handle rgb/rgba format
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

    return QColor(colorStr); // Try to convert directly
}

// Helper function to convert QColor to string in the appropriate format
QString MainWindow::colorToString(const QColor &color, const QString &originalFormat)
{
    if (originalFormat.startsWith("#")) {
        // Return hex format
        return color.name();
    } else if (originalFormat.startsWith("rgb(")) {
        // Return rgb format
        return QString("rgb(%1,%2,%3)").arg(color.red()).arg(color.green()).arg(color.blue());
    } else if (originalFormat.startsWith("rgba(")) {
        // Return rgba format
        return QString("rgba(%1,%2,%3,%4)").arg(color.red()).arg(color.green()).arg(color.blue()).arg(color.alpha());
    }

    return color.name(); // Default to hex
}

// Extract color from stylesheet
QString MainWindow::extractColor(const QString &styleSheet, const QString &property)
{
    int propIndex = styleSheet.indexOf(property);
    if (propIndex != -1) {
        int colonIndex = styleSheet.indexOf(':', propIndex);
        if (colonIndex != -1) {
            int endIndex = styleSheet.indexOf(';', colonIndex);
            if (endIndex != -1) {
                return styleSheet.mid(colonIndex + 1, endIndex - colonIndex - 1).trimmed();
            }
        }
    }
    return QString();
}

// Replace color in stylesheet
QString MainWindow::replaceColor(const QString &styleSheet, const QString &property, const QString &newColor)
{
    QString result = styleSheet;
    int propIndex = result.indexOf(property);

    while (propIndex != -1) {
        int colonIndex = result.indexOf(':', propIndex);
        if (colonIndex != -1) {
            int endIndex = result.indexOf(';', colonIndex);
            if (endIndex != -1) {
                result.replace(colonIndex + 1, endIndex - colonIndex - 1, newColor);
            }
        }
        propIndex = result.indexOf(property, propIndex + 1);
    }

    return result;
}

// Process and update colors in a stylesheet for light mode
QString MainWindow::processStyleSheetForLightMode(const QString &styleSheet)
{
    QString result = styleSheet;
    QStringList colorProperties = {
        "background", "background-color", "color", "border", "border-color",
        "gridline-color", "selection-background-color", "selection-color"
    };

    for (const QString &prop : colorProperties) {
        int propIndex = 0;
        while ((propIndex = result.indexOf(prop, propIndex)) != -1) {
            int colonIndex = result.indexOf(':', propIndex);
            if (colonIndex != -1) {
                int endIndex = result.indexOf(';', colonIndex);
                if (endIndex != -1) {
                    QString colorStr = result.mid(colonIndex + 1, endIndex - colonIndex - 1).trimmed();

                    // Skip if it's not actually a color
                    if (colorStr == "transparent" || colorStr == "none" ||
                        colorStr.contains("url") || colorStr.contains("gradient")) {
                        propIndex = endIndex + 1;
                        continue;
                    }

                    // Apply light mode color mapping
                    QString newColorStr = applyLightModeColor(colorStr);
                    if (!newColorStr.isEmpty()) {
                        result.replace(colonIndex + 1, endIndex - colonIndex - 1, " " + newColorStr);
                    }
                }
            }
            propIndex++;
        }
    }

    return result;
}

// Apply specific light mode color based on original color
QString MainWindow::applyLightModeColor(const QString &originalColor)
{
    // Clean color string for comparison
    QString cleanColor = originalColor.trimmed().toLower();

    // Special color mappings for light mode
    if (colorMappings.contains(cleanColor)) {
        return colorMappings[cleanColor];
    }

    // For colors not in our mapping, convert and process
    QColor color = stringToColor(cleanColor);
    if (color.isValid()) {
        // If it's a dark color (average < 128), make it lighter for light mode
        int average = (color.red() + color.green() + color.blue()) / 3;
        if (average < 128) {
            QColor lightColor;

            // Create pleasing light colors based on hue
            if (color.hue() == -1) {  // For grays
                int lightness = 255 - color.value();
                lightColor = QColor(lightness, lightness, lightness);
            } else {
                lightColor = QColor::fromHsv(
                    color.hue(),
                    qMax(0, color.saturation() - 40),  // Reduce saturation
                    qMin(255, color.value() + 150)     // Increase brightness
                    );
            }
            return colorToString(lightColor, cleanColor);
        }
    }

    return QString();  // Return empty if no processing needed
}


void MainWindow::enableDarkMode()
{
    if (!isDarkMode) {
        restoreDarkMode();
        isDarkMode = true;

        // Update button styles to show which mode is active
        ui->pushButton_83->setStyleSheet("max-height:37px; border-radius:15px; background:#262626; text-align:left; padding-left:20px;");
        ui->pushButton_84->setStyleSheet("max-height:37px; border:3px; background:#565656; border-radius:14px;");
    }
}

void MainWindow::applyLightModeToAllWidgets()
{
    // Initialize color mappings for light mode
    initializeColorMappings();

    // Store original stylesheets for restoration
    originalStylesheets.clear();

    // Process all widgets in the UI
    QList<QWidget*> allWidgets = this->findChildren<QWidget*>();

    for (QWidget* widget : allWidgets) {
        // Skip the sidebar frame
        if (widget == ui->frame_8 || widget->parent() == ui->frame_8 ||
            widget->objectName().startsWith("page2") ||
            widget->objectName().startsWith("label_24") ||
            widget->objectName().startsWith("pushButton_8")) {
            continue;
        }

        // Store original stylesheet
        originalStylesheets[widget] = widget->styleSheet();

        // Apply light mode colors
        QString styleSheet = widget->styleSheet();
        styleSheet = processStyleSheetForLightMode(styleSheet);
        widget->setStyleSheet(styleSheet);

        // Handle specific widget types
        if (qobject_cast<QTableWidget*>(widget)) {
            applyLightModeToTable(qobject_cast<QTableWidget*>(widget));
        }
    }

    // Store main window stylesheet
    originalMainStylesheet = this->styleSheet();

    // Apply light mode to main window
    QString mainStyleSheet = this->styleSheet();
    mainStyleSheet = processStyleSheetForLightMode(mainStyleSheet);
    this->setStyleSheet(mainStyleSheet);
}

void MainWindow::restoreDarkMode()
{
    // Restore original stylesheets
    for (auto it = originalStylesheets.begin(); it != originalStylesheets.end(); ++it) {
        QWidget* widget = it.key();
        if (widget) {
            widget->setStyleSheet(it.value());
        }
    }

    // Restore main window stylesheet
    this->setStyleSheet(originalMainStylesheet);
}



// void MainWindow::updateIcons()
// {
//     QString iconPath = isDarkTheme ? ":/icons/dark/" : ":/icons/light/";

// // Update sidebar icons
// ui->label_176->setIcon(QIcon(iconPath + "dashboard.svg"));
// ui->label_177->setIcon(QIcon(iconPath + "debt.svg"));
// ui->label_180->setIcon(QIcon(iconPath + "product.svg"));
// ui->label_181->setIcon(QIcon(iconPath + "vendor.svg"));
// ui->label_182->setIcon(QIcon(iconPath + "stock.svg"));
// ui->label_179->setIcon(QIcon(iconPath + "sales.svg"));
// ui->label_399->setIcon(QIcon(iconPath + "worker.svg"));
// ui->label_178->setIcon(QIcon(iconPath + "admin.svg"));
// ui->label_474->setIcon(QIcon(iconPath + "logout.svg"));

// Add theme toggle button icons
// ui->darkModeButton->setIcon(QIcon(iconPath + "moon.svg"));
// ui->lightModeButton->setIcon(QIcon(iconPath + "sun.svg"));

// Update search icon
// ui->searchIcon->setPixmap(QPixmap(iconPath + "search.svg"));
// }

void MainWindow::initializeColorMappings()
{
    // Clear existing mappings
    colorMappings.clear();

    // Define refined light mode color palette
    // Main backgrounds - subtle gradient from white to light blue-gray
    colorMappings["#0a0a0a"] = "#ffffff";    // Pure white for main backgrounds
    colorMappings["#121212"] = "#f8fafc";    // Very slight blue tint for secondary backgrounds
    colorMappings["#181818"] = "#f1f5f9";    // Light blue-gray for containers
    colorMappings["#262626"] = "#e2e8f0";    // Light slate for inputs
    colorMappings["#3a3a3a"] = "#cbd5e1";    // Light blue-gray for borders
    colorMappings["#565656"] = "#94a3b8";    // Medium slate for separators
    colorMappings["#adadad"] = "#64748b";    // Medium slate blue for secondary text
    colorMappings["#626262"] = "#475569";    // Slate blue for text
    colorMappings["#fcfcfc"] = "#1e293b";    // Dark slate for primary text

    // Modern accent colors
    colorMappings["#436cfd"] = "#3b82f6";    // Vibrant blue
    colorMappings["#0078d7"] = "#2563eb";    // Darker blue for hover states

    // Text colors for better readability
    colorMappings["#494949"] = "#334155";    // Dark slate text
    colorMappings["white"] = "#0f172a";      // Almost black text with slight blue tint
    colorMappings["#e0e4e4"] = "#475569";    // Slate gray text

    // Professional accent colors for UI elements
    colorMappings["#4fc3f7"] = "#38bdf8";    // Info blue
    colorMappings["#66bb6a"] = "#10b981";    // Success green
    colorMappings["#ff7043"] = "#f97316";    // Warning orange
    colorMappings["#ef5350"] = "#ef4444";    // Danger red
}

void MainWindow::applyLightModeToTable(QTableWidget* table)
{
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

    // Apply refined header styling
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

    // Apply vertical header styling
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

    // Set table grid style
    table->setShowGrid(true);
    table->setGridStyle(Qt::SolidLine);

    // Apply table style
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

    // Apply alternating row colors for better readability
    table->setAlternatingRowColors(true);

    // Apply custom colors to items
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

void MainWindow::enableLightMode()
{
    if (isDarkMode) {
        applyLightModeToAllWidgets();
        isDarkMode = false;

        // Apply enhanced scrollbar styling
        QString scrollbarStyle =
            "QScrollBar:vertical {"
            "    background: #f1f5f9;"
            "    width: 10px;"
            "    margin: 0px;"
            "}"
            "QScrollBar::handle:vertical {"
            "    background: #94a3b8;"
            "    min-height: 20px;"
            "    border-radius: 5px;"
            "}"
            "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
            "    height: 0px;"
            "}"
            "QScrollBar:horizontal {"
            "    background: #f1f5f9;"
            "    height: 10px;"
            "    margin: 0px;"
            "}"
            "QScrollBar::handle:horizontal {"
            "    background: #94a3b8;"
            "    min-width: 20px;"
            "    border-radius: 5px;"
            "}"
            "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {"
            "    width: 0px;"
            "}";

        // Apply to main application and all scrollable widgets
        qApp->setStyleSheet(qApp->styleSheet() + scrollbarStyle);

        // Enhanced button styling
        QString buttonStyle =
            "QPushButton {"
            "    background-color: #f1f5f9;"
            "    color: #334155;"
            "    border: 1px solid #cbd5e1;"
            "    border-radius: 4px;"
            "    padding: 5px 10px;"
            "}"
            "QPushButton:hover {"
            "    background-color: #e2e8f0;"
            "    border-color: #94a3b8;"
            "}"
            "QPushButton:pressed {"
            "    background-color: #cbd5e1;"
            "}";

        // Apply button styles globally (excluding special buttons we don't want to affect)
        for (QPushButton* button : this->findChildren<QPushButton*>()) {
            if (button->objectName() != "pushButton_83" &&
                button->objectName() != "pushButton_84" &&
                !button->objectName().startsWith("page")) {

                button->setStyleSheet(button->styleSheet() + buttonStyle);
            }
        }

        // Special styling for the light/dark mode switcher buttons
        ui->pushButton_83->setStyleSheet(
            "max-height:37px;"
            "border-radius:15px;"
            "background:#3b82f6;"
            "text-align:left;"
            "padding-left:20px;"
            "color:#ffffff;"
            "font-weight:bold;"
            );

        ui->pushButton_84->setStyleSheet(
            "max-height:37px;"
            "border:1px solid #cbd5e1;"
            "background:#f8fafc;"
            "border-radius:14px;"
            "color:#64748b;"
            );

        // Apply enhanced line edit styling
        QString lineEditStyle =
            "QLineEdit {"
            "    background-color: #ffffff;"
            "    color: #334155;"
            "    border: 1px solid #cbd5e1;"
            "    border-radius: 4px;"
            "    padding: 5px;"
            "}"
            "QLineEdit:focus {"
            "    border: 1px solid #3b82f6;"
            "}";

        for (QLineEdit* lineEdit : this->findChildren<QLineEdit*>()) {
            lineEdit->setStyleSheet(lineEdit->styleSheet() + lineEditStyle);
        }
    }
}

// Additional method to apply soft shadows to important UI elements
void MainWindow::applyLightModeEffects()
{
    // Apply shadow effects to frames and groupboxes
    for (QFrame* frame : this->findChildren<QFrame*>()) {
        // Skip sidebar frame
        if (frame == ui->frame_8 || frame->parent() == ui->frame_8) {
            continue;
        }

        // Apply shadow effect to frames that should stand out
        if (frame->frameShape() == QFrame::StyledPanel ||
            frame->frameShape() == QFrame::Box ||
            frame->frameShape() == QFrame::Panel) {

            QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect;
            shadow->setBlurRadius(15);
            shadow->setColor(QColor(0, 0, 0, 25)); // Transparent black
            shadow->setOffset(0, 2);
            frame->setGraphicsEffect(shadow);
        }
    }

    // Apply shadow to groupboxes
    for (QGroupBox* box : this->findChildren<QGroupBox*>()) {
        QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect;
        shadow->setBlurRadius(10);
        shadow->setColor(QColor(0, 0, 0, 20)); // Transparent black
        shadow->setOffset(0, 1);
        box->setGraphicsEffect(shadow);
    }
}







