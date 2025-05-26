#include "salesdashboard.h"
#include "databasehandler.h"
#include "productmanager.h"
#include "salesmanager.h"
#include "clickableWidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDebug>
#include <QPushButton>
#include <QScrollArea>

SalesDashboard::SalesDashboard(DatabaseHandler *dbHandler, ProductManager *productManager,
                               SalesManager *salesManager, QWidget *parent)
    : QWidget(parent), m_dbHandler(dbHandler), m_productManager(productManager)
    , m_salesManager(salesManager), m_totalAmount(0.0), m_currentSelectedRow(-1)
{
    setupUI();
    connectSignals();
    refreshData();
}

SalesDashboard::~SalesDashboard() = default;

void SalesDashboard::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    auto *contentLayout = new QHBoxLayout();

    // Left panel
    auto *leftLayout = new QVBoxLayout();
    setupProductSearch();
    leftLayout->addWidget(m_productSearchEdit);

    // Recommendations
    m_recommendLayout = new QVBoxLayout();
    auto *recommendScroll = new QScrollArea(this);
    auto *recommendWidget = new QWidget(this);
    recommendWidget->setLayout(m_recommendLayout);
    recommendScroll->setWidget(recommendWidget);
    recommendScroll->setWidgetResizable(true);
    leftLayout->addWidget(recommendScroll, 3);

    // Debug tables (hidden)
    setupDebugTables();
    leftLayout->addWidget(m_productsTable);
    leftLayout->addWidget(m_salesSearchEdit);
    leftLayout->addWidget(m_salesTable);

    // Right panel
    auto *rightLayout = new QVBoxLayout();
    setupRightPanel(rightLayout);

    contentLayout->addLayout(leftLayout, 3);
    contentLayout->addLayout(rightLayout, 2);
    mainLayout->addLayout(contentLayout);
}

void SalesDashboard::setupProductSearch()
{
    m_productSearchEdit = new QLineEdit(this);
    m_productSearchEdit->setPlaceholderText("Search Products...");
    m_productSearchEdit->setStyleSheet("padding: 8px; border-radius: 4px; font-size: 14px;");
}

void SalesDashboard::setupDebugTables()
{
    // Products table
    m_productsTable = new QTableWidget(this);
    m_productsTable->setColumnCount(6);
    m_productsTable->setHorizontalHeaderLabels({"ID", "Name", "Price", "Category", "Stock", "Updated"});
    m_productsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_productsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_productsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_productsTable->verticalHeader()->setVisible(false);
    m_productsTable->setVisible(false);

    // Sales search and table
    m_salesSearchEdit = new QLineEdit(this);
    m_salesSearchEdit->setPlaceholderText("Search Sales...");

    m_salesTable = new QTableWidget(this);
    m_salesTable->setColumnCount(8);
    m_salesTable->setHorizontalHeaderLabels({
        "Sales ID", "Salesman ID", "Product ID", "Product Name",
        "Price", "Category", "Quantity Sold", "Date/Time"
    });
    m_salesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_salesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_salesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_salesTable->verticalHeader()->setVisible(false);
    m_salesTable->setVisible(false);
}

void SalesDashboard::setupRightPanel(QVBoxLayout *rightLayout)
{
    // Title
    auto *selectedTitle = new QLabel("Selected Products", this);
    selectedTitle->setStyleSheet("font-size: 16px; font-weight: bold;");
    rightLayout->addWidget(selectedTitle);

    // Selected products scroll
    m_selectedLayout = new QVBoxLayout();
    auto *selectedScroll = new QScrollArea(this);
    auto *selectedWidget = new QWidget(this);
    selectedWidget->setLayout(m_selectedLayout);
    selectedScroll->setWidget(selectedWidget);
    selectedScroll->setWidgetResizable(true);
    rightLayout->addWidget(selectedScroll, 3);

    // Selected products table
    m_selectedProductsTable = new QTableWidget(this);
    m_selectedProductsTable->setColumnCount(5);
    m_selectedProductsTable->setHorizontalHeaderLabels({"Product", "Price", "Qty", "Total", "Remove"});
    m_selectedProductsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_selectedProductsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_selectedProductsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_selectedProductsTable->verticalHeader()->setVisible(false);
    rightLayout->addWidget(m_selectedProductsTable, 3);

    // Controls
    setupControls(rightLayout);
}

void SalesDashboard::setupControls(QVBoxLayout *rightLayout)
{
    // Quantity controls
    auto *qtyLayout = new QHBoxLayout();
    qtyLayout->addWidget(new QLabel("Quantity:", this));
    qtyLayout->addStretch();

    m_removeQtyBtn = new QPushButton("-", this);
    m_addQtyBtn = new QPushButton("+", this);
    QString btnStyle = "background-color: #436cfd; color: white; font-weight: bold; padding: 5px 10px;";
    m_addQtyBtn->setStyleSheet(btnStyle);
    m_removeQtyBtn->setStyleSheet(btnStyle);

    qtyLayout->addWidget(m_removeQtyBtn);
    qtyLayout->addWidget(m_addQtyBtn);
    rightLayout->addLayout(qtyLayout);

    // Total
    auto *totalLayout = new QHBoxLayout();
    totalLayout->addWidget(new QLabel("Total:", this));
    totalLayout->addStretch();

    m_totalLabel = new QLabel("0.00 Rs.", this);
    m_totalLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_totalLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    totalLayout->addWidget(m_totalLabel);
    rightLayout->addLayout(totalLayout);

    // Action buttons
    auto *actionLayout = new QHBoxLayout();
    m_clearSelectionBtn = new QPushButton("Clear Selection", this);
    m_sellProductsBtn = new QPushButton("Complete Sale", this);

    m_clearSelectionBtn->setStyleSheet("background-color: #e74c3c; color: white; padding: 10px; font-weight: bold;");
    m_sellProductsBtn->setStyleSheet("background-color: #2ecc71; color: white; padding: 10px; font-weight: bold;");
    m_clearSelectionBtn->setMinimumHeight(40);
    m_sellProductsBtn->setMinimumHeight(40);

    actionLayout->addWidget(m_clearSelectionBtn);
    actionLayout->addWidget(m_sellProductsBtn);
    rightLayout->addLayout(actionLayout);
}

void SalesDashboard::connectSignals()
{
    // Search signals
    connect(m_productSearchEdit, &QLineEdit::textChanged, this, &SalesDashboard::onProductSearchTextChanged);
    connect(m_salesSearchEdit, &QLineEdit::textChanged, this, &SalesDashboard::onSalesSearchTextChanged);

    // Table selection signals
    connect(m_productsTable, &QTableWidget::cellClicked, this, &SalesDashboard::onProductSelected);
    connect(m_selectedProductsTable, &QTableWidget::cellClicked, this, &SalesDashboard::onSelectedProductClicked);

    // Button signals - Use Qt::QueuedConnection to prevent double execution
    connect(m_sellProductsBtn, &QPushButton::clicked, this, &SalesDashboard::onSellProductsClicked, Qt::QueuedConnection);
    connect(m_clearSelectionBtn, &QPushButton::clicked, this, &SalesDashboard::onClearSelectionClicked, Qt::QueuedConnection);
    // connect(m_addQtyBtn, &QPushButton::clicked, [this](bool) { onQuantityChanged(true); }, Qt::QueuedConnection);
    // connect(m_removeQtyBtn, &QPushButton::clicked, [this](bool) { onQuantityChanged(false); }, Qt::QueuedConnection);


    // SalesManager signals
    connect(m_salesManager, &SalesManager::productSelectedFromWidget, this, &SalesDashboard::onProductSelectedFromWidget);
}

void SalesDashboard::refreshData()
{
    if (m_productsTable->isVisible()) m_productManager->loadProducts(m_productsTable);
    if (m_salesTable->isVisible()) m_salesManager->loadSales(m_salesTable);
}

void SalesDashboard::clearLayout(QLayout *layout)
{
    if (!layout) return;
    QLayoutItem *item;
    while ((item = layout->takeAt(0))) {
        delete item->widget();
        delete item;
    }
}

void SalesDashboard::refreshProductList()
{
    clearLayout(m_recommendLayout);
    refreshData();
}

void SalesDashboard::refreshSalesTable()
{
    if (m_salesTable->isVisible()) m_salesManager->loadSales(m_salesTable);
}

void SalesDashboard::resetSalesArea()
{
    m_selectedItems.clear();
    m_selectedProductsTable->setRowCount(0);
    clearLayout(m_selectedLayout);
    m_totalAmount = 0.0;
    m_totalLabel->setText("0.00 Rs.");
    m_currentSelectedRow = -1;
}

void SalesDashboard::onProductSearchTextChanged(const QString &text)
{
    clearLayout(m_recommendLayout);

    if (text.isEmpty()) return;

    // Use existing manager method for recommendations
    m_salesManager->getProductsForRecommendation(m_recommendLayout, text);

    // Update debug table if visible
    if (m_productsTable->isVisible()) {
        m_productManager->searchProducts(m_productsTable, text);
    }
}

void SalesDashboard::onSalesSearchTextChanged(const QString &text)
{
    if (m_salesTable->isVisible()) {
        m_salesManager->searchSales(m_salesTable, text);
    }
}

void SalesDashboard::onProductSelected(int row, int column)
{
    if (row < 0 || row >= m_productsTable->rowCount()) return;

    SaleItem item;
    item.productId = m_productsTable->item(row, 0)->text().toInt();
    item.productName = m_productsTable->item(row, 1)->text();
    item.unitPrice = m_productsTable->item(row, 2)->text().toDouble();
    item.category = m_productsTable->item(row, 3)->text();
    item.available = m_productsTable->item(row, 4)->text().toInt();
    item.quantity = 1;
    item.totalPrice = item.unitPrice;

    // Check stock before adding
    if (item.available <= 0) {
        QMessageBox::warning(this, "Out of Stock",
                             QString("Product '%1' is currently out of stock.").arg(item.productName));
        return;
    }

    addProductToSelectedList(item);
}

void SalesDashboard::onProductSelectedFromWidget(int productId, QString productName, double price, QString category, int available)
{
    // This method handles clicks from the recommendation widgets
    SaleItem item;
    item.productId = productId;
    item.productName = productName;
    item.unitPrice = price;
    item.category = category;
    item.available = available;
    item.quantity = 1;
    item.totalPrice = price;

    // Check stock before adding
    if (item.available <= 0) {
        QMessageBox::warning(this, "Out of Stock",
                             QString("Product '%1' is currently out of stock.").arg(item.productName));
        return;
    }

    addProductToSelectedList(item);
}

void SalesDashboard::onSelectedProductClicked(int row, int column)
{
    highlightSelectedProduct(row);
}

void SalesDashboard::highlightSelectedProduct(int row)
{
    // Clear previous highlight
    if (m_currentSelectedRow >= 0 && m_currentSelectedRow < m_selectedProductsTable->rowCount()) {
        for (int col = 0; col < m_selectedProductsTable->columnCount(); col++) {
            auto *item = m_selectedProductsTable->item(m_currentSelectedRow, col);
            if (item) item->setBackground(Qt::transparent);
        }
    }

    // Set new highlight
    if (row >= 0 && row < m_selectedProductsTable->rowCount()) {
        for (int col = 0; col < m_selectedProductsTable->columnCount(); col++) {
            auto *item = m_selectedProductsTable->item(row, col);
            if (item) item->setBackground(QColor("#436cfd"));
        }
    }

    m_currentSelectedRow = row;
}

void SalesDashboard::addProductToSelectedList(const SaleItem &item)
{
    // Check for existing product - if exists, increase quantity instead of duplicating
    for (int i = 0; i < m_selectedItems.size(); i++) {
        if (m_selectedItems[i].productId == item.productId) {
            updateSelectedItemQuantity(i, true);
            return;
        }
    }

    // Add new product
    m_selectedItems.append(item);
    int row = m_selectedProductsTable->rowCount();
    m_selectedProductsTable->insertRow(row);

    // Populate table row
    m_selectedProductsTable->setItem(row, 0, new QTableWidgetItem(item.productName));
    m_selectedProductsTable->setItem(row, 1, new QTableWidgetItem(QString::number(item.unitPrice, 'f', 2)));
    m_selectedProductsTable->setItem(row, 2, new QTableWidgetItem(QString::number(item.quantity)));
    m_selectedProductsTable->setItem(row, 3, new QTableWidgetItem(QString::number(item.totalPrice, 'f', 2)));

    // Add remove button
    auto *removeBtn = new QPushButton("X", this);
    removeBtn->setCursor(Qt::PointingHandCursor);
    removeBtn->setStyleSheet("background-color: #e74c3c; color: white; font-weight: bold;");

    // Use a queued connection to prevent double execution
    // connect(removeBtn, &QPushButton::clicked, [this, row]() {
    //     removeProduct(row);
    // }, Qt::QueuedConnection);

    m_selectedProductsTable->setCellWidget(row, 4, removeBtn);

    updateTotalAmount();
    highlightSelectedProduct(row);
}

void SalesDashboard::removeProduct(int row)
{
    if (row >= 0 && row < m_selectedItems.size()) {
        m_selectedItems.removeAt(row);
        m_selectedProductsTable->removeRow(row);
        updateTotalAmount();

        // Update current selection tracking
        if (m_currentSelectedRow == row) {
            m_currentSelectedRow = -1;
        } else if (m_currentSelectedRow > row) {
            m_currentSelectedRow--;
        }
    }
}

void SalesDashboard::updateSelectedItemQuantity(int index, bool increase)
{
    if (index < 0 || index >= m_selectedItems.size()) return;

    auto &item = m_selectedItems[index];

    if (increase) {
        if (item.quantity < item.available) {
            item.quantity++;
        } else {
            QMessageBox::warning(this, "Insufficient Stock",
                                 QString("Cannot add more of '%1'. Only %2 units available in stock.")
                                     .arg(item.productName).arg(item.available));
            return;
        }
    } else {
        if (item.quantity > 1) {
            item.quantity--;
        } else {
            if (item.quantity > 1) {
                item.quantity--;
            } else {
                return;
            }
        }
    }

    item.totalPrice = item.quantity * item.unitPrice;

    // Update table
    m_selectedProductsTable->item(index, 2)->setText(QString::number(item.quantity));
    m_selectedProductsTable->item(index, 3)->setText(QString::number(item.totalPrice, 'f', 2));

    updateTotalAmount();
}

void SalesDashboard::updateTotalAmount()
{
    m_totalAmount = 0.0;
    for (const auto &item : m_selectedItems) {
        m_totalAmount += item.totalPrice;
    }
    m_totalLabel->setText(QString("%1 Rs.").arg(m_totalAmount, 0, 'f', 2));
}

void SalesDashboard::onQuantityChanged(bool increase)
{
    if (m_currentSelectedRow >= 0 && m_currentSelectedRow < m_selectedItems.size()) {
        updateSelectedItemQuantity(m_currentSelectedRow, increase);
    }
}

void SalesDashboard::onSellProductsClicked()
{
    // Prevent double execution
    static bool processing = false;
    if (processing) return;
    processing = true;

    if (m_selectedItems.isEmpty()) {
        QMessageBox::warning(this, "No Products Selected",
                             "Please select at least one product to complete the sale.");
        processing = false;
        return;
    }

    // Validate stock for all items before processing
    for (const auto &item : m_selectedItems) {
        QSqlQuery stockCheck;
        stockCheck.prepare("SELECT quantity FROM Products WHERE product_id = ?");
        stockCheck.addBindValue(item.productId);

        if (!stockCheck.exec() || !stockCheck.next()) {
            QMessageBox::critical(this, "Stock Check Failed",
                                  QString("Failed to verify stock for product '%1'.").arg(item.productName));
            processing = false;
            return;
        }

        int availableStock = stockCheck.value(0).toInt();
        if (availableStock < item.quantity) {
            QMessageBox::warning(this, "Insufficient Stock",
                                 QString("Product '%1' only has %2 units in stock, but you're trying to sell %3 units.")
                                     .arg(item.productName).arg(availableStock).arg(item.quantity));
            processing = false;
            return;
        }
    }

    int userId = m_dbHandler->getCurrentUserId();
    if (userId <= 0) {
        QMessageBox::critical(this, "Authentication Error",
                              "User information not available. Please log in again.");
        processing = false;
        return;
    }

    if (m_salesManager->processSale(m_selectedItems, userId)) {
        QMessageBox::information(this, "Sale Completed",
                                 QString("Sale of %1 items totaling %2 Rs. has been successfully recorded.")
                                     .arg(m_selectedItems.size())
                                     .arg(m_totalAmount, 0, 'f', 2));
        resetSalesArea();
        refreshSalesTable();
        refreshProductList(); // Refresh to update stock counts
    } else {
        QMessageBox::critical(this, "Sale Failed",
                              "There was an error processing the sale. Please try again.");
    }

    processing = false;
}

void SalesDashboard::onClearSelectionClicked()
{
    // Prevent double execution
    static bool processing = false;
    if (processing) return;
    processing = true;

    if (m_selectedItems.isEmpty()) {
        processing = false;
        return;
    }

    auto reply = QMessageBox::question(this, "Clear Selection",
                                       "Are you sure you want to clear all selected products?",
                                       QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        resetSalesArea();
    }

    processing = false;
}
