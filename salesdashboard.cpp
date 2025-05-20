#include "salesdashboard.h"
#include "databasehandler.h"
#include "productmanager.h"
#include "salesmanager.h"
#include "authmanager.h"
#include "clickableWidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDebug>
#include <QDateTime>
#include <QPushButton>
#include <QScrollArea>

SalesDashboard::SalesDashboard(DatabaseHandler *dbHandler,
                               ProductManager *productManager,
                               SalesManager *salesManager,
                               AuthManager *authManager,
                               QWidget *parent)
    : QWidget(parent)
    , m_dbHandler(dbHandler)
    , m_productManager(productManager)
    , m_salesManager(salesManager)
    , m_authManager(authManager)
    , m_totalAmount(0.0)
    , m_currentSelectedRow(-1)
{
    setupUI();

    // Connect signals and slots
    connect(m_productSearchEdit, &QLineEdit::textChanged, this, &SalesDashboard::onProductSearchTextChanged);
    connect(m_salesSearchEdit, &QLineEdit::textChanged, this, &SalesDashboard::onSalesSearchTextChanged);
    connect(m_productsTable, &QTableWidget::cellClicked, this, &SalesDashboard::onProductSelected);
    connect(m_selectedProductsTable, &QTableWidget::cellClicked, this, &SalesDashboard::onSelectedProductClicked);
    connect(m_sellProductsBtn, &QPushButton::clicked, this, &SalesDashboard::onSellProductsClicked);
    connect(m_clearSelectionBtn, &QPushButton::clicked, this, &SalesDashboard::onClearSelectionClicked);
    connect(m_addQtyBtn, &QPushButton::clicked, this, [this]() { onQuantityChanged(true); });
    connect(m_removeQtyBtn, &QPushButton::clicked, this, [this]() { onQuantityChanged(false); });

    // Initial data load
    refreshProductList();
    refreshSalesTable();
}

SalesDashboard::~SalesDashboard()
{
    // Clean up any dynamic allocations if needed
}

void SalesDashboard::setupUI()
{
    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Create horizontal layout for main content
    QHBoxLayout *contentLayout = new QHBoxLayout();

    // Left panel - Search and product recommendations
    QVBoxLayout *leftLayout = new QVBoxLayout();

    // Product search
    setupProductSearch();
    leftLayout->addWidget(m_productSearchEdit);

    // Recommended products layout
    m_recommendLayout = new QVBoxLayout();
    QScrollArea *recommendScroll = new QScrollArea(this);
    recommendScroll->setWidgetResizable(true);
    QWidget *recommendWidget = new QWidget(this);
    recommendWidget->setLayout(m_recommendLayout);
    recommendScroll->setWidget(recommendWidget);
    leftLayout->addWidget(recommendScroll, 3);

    // Products table (for testing/debugging)
    setupProductsTable();
    leftLayout->addWidget(m_productsTable);

    // Sales search and table
    m_salesSearchEdit = new QLineEdit(this);
    m_salesSearchEdit->setPlaceholderText("Search Sales...");
    leftLayout->addWidget(m_salesSearchEdit);

    setupSalesTable();
    leftLayout->addWidget(m_salesTable);

    // Right panel - Selected products and actions
    QVBoxLayout *rightLayout = new QVBoxLayout();

    // Selected products title
    QLabel *selectedTitle = new QLabel("Selected Products", this);
    selectedTitle->setStyleSheet("font-size: 16px; font-weight: bold;");
    rightLayout->addWidget(selectedTitle);

    // Selected products layout
    m_selectedLayout = new QVBoxLayout();
    QScrollArea *selectedScroll = new QScrollArea(this);
    selectedScroll->setWidgetResizable(true);
    QWidget *selectedWidget = new QWidget(this);
    selectedWidget->setLayout(m_selectedLayout);
    selectedScroll->setWidget(selectedWidget);
    rightLayout->addWidget(selectedScroll, 3);

    // Selected products table (for functionality)
    setupSelectedProductsTable();
    rightLayout->addWidget(m_selectedProductsTable, 3);

    // Quantity controls
    QHBoxLayout *qtyLayout = new QHBoxLayout();
    QLabel *qtyLabel = new QLabel("Quantity:", this);
    m_addQtyBtn = new QPushButton("+", this);
    m_removeQtyBtn = new QPushButton("-", this);

    // Style the buttons
    m_addQtyBtn->setStyleSheet("background-color: #436cfd; color: white; font-weight: bold; padding: 5px 10px;");
    m_removeQtyBtn->setStyleSheet("background-color: #436cfd; color: white; font-weight: bold; padding: 5px 10px;");

    qtyLayout->addWidget(qtyLabel);
    qtyLayout->addStretch();
    qtyLayout->addWidget(m_removeQtyBtn);
    qtyLayout->addWidget(m_addQtyBtn);
    rightLayout->addLayout(qtyLayout);

    // Total amount
    QHBoxLayout *totalLayout = new QHBoxLayout();
    QLabel *totalTextLabel = new QLabel("Total:", this);
    m_totalLabel = new QLabel("0.00 Rs.", this);
    m_totalLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_totalLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    totalLayout->addWidget(totalTextLabel);
    totalLayout->addStretch();
    totalLayout->addWidget(m_totalLabel);
    rightLayout->addLayout(totalLayout);

    // Action buttons
    setupActionButtons();
    QHBoxLayout *actionLayout = new QHBoxLayout();
    actionLayout->addWidget(m_clearSelectionBtn);
    actionLayout->addWidget(m_sellProductsBtn);
    rightLayout->addLayout(actionLayout);

    // Add left and right panels to the content layout
    contentLayout->addLayout(leftLayout, 3);
    contentLayout->addLayout(rightLayout, 2);

    // Add content layout to main layout
    mainLayout->addLayout(contentLayout);

    setLayout(mainLayout);
}

void SalesDashboard::setupProductSearch()
{
    m_productSearchEdit = new QLineEdit(this);
    m_productSearchEdit->setPlaceholderText("Search Products...");
    m_productSearchEdit->setStyleSheet("padding: 8px; border-radius: 4px; font-size: 14px;");
}

void SalesDashboard::setupProductsTable()
{
    m_productsTable = new QTableWidget(this);
    m_productsTable->setColumnCount(6);
    m_productsTable->setHorizontalHeaderLabels({"ID", "Name", "Price", "Category", "Stock", "Updated"});
    m_productsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_productsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_productsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_productsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_productsTable->verticalHeader()->setVisible(false);

    // Hide this table in production (just used for debugging)
    m_productsTable->setVisible(false);
}

void SalesDashboard::setupSelectedProductsTable()
{
    m_selectedProductsTable = new QTableWidget(this);
    m_selectedProductsTable->setColumnCount(5);
    m_selectedProductsTable->setHorizontalHeaderLabels({"Product", "Price", "Qty", "Total", "Remove"});
    m_selectedProductsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_selectedProductsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_selectedProductsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_selectedProductsTable->verticalHeader()->setVisible(false);
}

void SalesDashboard::setupSalesTable()
{
    m_salesTable = new QTableWidget(this);
    m_salesTable->setColumnCount(8);
    m_salesTable->setHorizontalHeaderLabels({
        "Sales ID", "Salesman ID", "Product ID",
        "Product Name", "Price", "Category",
        "Quantity Sold", "Date/Time"
    });
    m_salesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_salesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_salesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_salesTable->verticalHeader()->setVisible(false);

    // Hide this table in production (used for debugging)
    m_salesTable->setVisible(false);
}

void SalesDashboard::setupActionButtons()
{
    m_clearSelectionBtn = new QPushButton("Clear Selection", this);
    m_sellProductsBtn = new QPushButton("Complete Sale", this);

    // Style the buttons
    m_clearSelectionBtn->setStyleSheet("background-color: #e74c3c; color: white; padding: 10px; font-weight: bold;");
    m_sellProductsBtn->setStyleSheet("background-color: #2ecc71; color: white; padding: 10px; font-weight: bold;");

    // Set minimum height
    m_clearSelectionBtn->setMinimumHeight(40);
    m_sellProductsBtn->setMinimumHeight(40);
}

void SalesDashboard::refreshProductList()
{
    // Clear existing recommendations
    QLayoutItem *item;
    while ((item = m_recommendLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }

    // Load products from database (for testing)
    if (m_productsTable->isVisible()) {
        m_productManager->loadProducts(m_productsTable);
    }
}

void SalesDashboard::refreshSalesTable()
{
    // Load sales from database
    if (m_salesTable->isVisible()) {
        m_salesManager->loadSales(m_salesTable);
    }
}

void SalesDashboard::resetSalesArea()
{
    // Clear selected products
    m_selectedItems.clear();
    m_selectedProductsTable->setRowCount(0);

    // Clear selected products in UI
    QLayoutItem *item;
    while ((item = m_selectedLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }

    // Reset total amount
    m_totalAmount = 0.0;
    m_totalLabel->setText(QString("%1 Rs.").arg(m_totalAmount, 0, 'f', 2));

    // Reset current selection
    m_currentSelectedRow = -1;
}

void SalesDashboard::onProductSearchTextChanged(const QString &text)
{
    // Clear existing recommendations
    QLayoutItem *item;
    while ((item = m_recommendLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }

    if (text.isEmpty()) {
        return;
    }

    // Search for products in the database
    QSqlQuery query;
    query.prepare("SELECT product_id, product_name, price, category, quantity "
                  "FROM Products WHERE product_name LIKE :search "
                  "ORDER BY product_name LIMIT 10");
    query.bindValue(":search", "%" + text + "%");

    if (!query.exec()) {
        qDebug() << "Failed to search products:" << query.lastError().text();
        return;
    }

    // Create product recommendation widgets
    while (query.next()) {
        int productId = query.value("product_id").toInt();
        QString name = query.value("product_name").toString();
        double price = query.value("price").toDouble();
        QString category = query.value("category").toString();
        int quantity = query.value("quantity").toInt();

        // Create product widget
        ClickableWidget *productWidget = new ClickableWidget(this);

        productWidget->setObjectName(QString("product_%1").arg(productId));
        productWidget->setStyleSheet("background-color: #1e1e1e; color: white; border-radius: 5px; margin: 2px;");
        productWidget->setCursor(Qt::PointingHandCursor);

        QHBoxLayout *productLayout = new QHBoxLayout(productWidget);
        productLayout->setContentsMargins(10, 5, 10, 5);

        QVBoxLayout *infoLayout = new QVBoxLayout();
        QLabel *nameLabel = new QLabel(name, productWidget);
        nameLabel->setStyleSheet("font-size: 14px; font-weight: bold;");
        QLabel *unitLabel = new QLabel(QString("1.00 Units at %1 Rs. / Unit").arg(price, 0, 'f', 2), productWidget);
        unitLabel->setStyleSheet("font-size: 12px; color: #aaa;");

        infoLayout->addWidget(nameLabel);
        infoLayout->addWidget(unitLabel);

        QLabel *priceLabel = new QLabel(QString("%1 Rs.").arg(price, 0, 'f', 2), productWidget);
        priceLabel->setStyleSheet("font-size: 14px; font-weight: bold;");
        priceLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

        productLayout->addLayout(infoLayout);
        productLayout->addStretch();
        productLayout->addWidget(priceLabel);

        // Store product data as property
        productWidget->setProperty("productId", productId);
        productWidget->setProperty("productName", name);
        productWidget->setProperty("unitPrice", price);
        productWidget->setProperty("category", category);
        productWidget->setProperty("available", quantity);

        // Connect click event
        connect(productWidget, &ClickableWidget::clicked, [this, productWidget]() {
            SaleItem item;
            item.productId = productWidget->property("productId").toInt();
            item.productName = productWidget->property("productName").toString();
            item.unitPrice = productWidget->property("unitPrice").toDouble();
            item.category = productWidget->property("category").toString();
            item.available = productWidget->property("available").toInt();
            item.quantity = 1;
            item.totalPrice = item.unitPrice;

            addProductToSelectedList(item);
        });

        m_recommendLayout->addWidget(productWidget);
    }

    // Update product table (for testing)
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
    // Get product data from the table
    SaleItem item;
    item.productId = m_productsTable->item(row, 0)->text().toInt();
    item.productName = m_productsTable->item(row, 1)->text();
    item.unitPrice = m_productsTable->item(row, 2)->text().toDouble();
    item.category = m_productsTable->item(row, 3)->text();
    item.available = m_productsTable->item(row, 4)->text().toInt();
    item.quantity = 1;
    item.totalPrice = item.unitPrice;

    // Add product to selected list
    addProductToSelectedList(item);
}

void SalesDashboard::onSelectedProductClicked(int row, int column)
{
    // Highlight the selected row
    highlightSelectedProduct(row);
}

void SalesDashboard::highlightSelectedProduct(int row)
{
    // Reset previous selection
    if (m_currentSelectedRow >= 0 && m_currentSelectedRow < m_selectedProductsTable->rowCount()) {
        for (int col = 0; col < m_selectedProductsTable->columnCount(); col++) {
            if (m_selectedProductsTable->item(m_currentSelectedRow, col)) {
                m_selectedProductsTable->item(m_currentSelectedRow, col)->setBackground(Qt::transparent);
            }
        }
    }

    // Highlight new selection
    if (row >= 0 && row < m_selectedProductsTable->rowCount()) {
        for (int col = 0; col < m_selectedProductsTable->columnCount(); col++) {
            if (m_selectedProductsTable->item(row, col)) {
                m_selectedProductsTable->item(row, col)->setBackground(QColor("#436cfd"));
            }
        }
    }

    m_currentSelectedRow = row;
}

void SalesDashboard::addProductToSelectedList(const SaleItem &item)
{
    // Check if product is already in the list
    for (int i = 0; i < m_selectedItems.size(); i++) {
        if (m_selectedItems[i].productId == item.productId) {
            // Increase quantity instead of adding again
            updateSelectedItemQuantity(i, true);
            return;
        }
    }

    // Add to internal list
    m_selectedItems.append(item);

    // Add to UI list
    int row = m_selectedProductsTable->rowCount();
    m_selectedProductsTable->insertRow(row);

    m_selectedProductsTable->setItem(row, 0, new QTableWidgetItem(item.productName));
    m_selectedProductsTable->setItem(row, 1, new QTableWidgetItem(QString::number(item.unitPrice, 'f', 2)));
    m_selectedProductsTable->setItem(row, 2, new QTableWidgetItem(QString::number(item.quantity)));
    m_selectedProductsTable->setItem(row, 3, new QTableWidgetItem(QString::number(item.totalPrice, 'f', 2)));

    // Add remove button
    QPushButton *removeBtn = new QPushButton("X", this);
    removeBtn->setCursor(Qt::PointingHandCursor);
    removeBtn->setStyleSheet("background-color: #e74c3c; color: white; font-weight: bold;");
    connect(removeBtn, &QPushButton::clicked, [this, row]() {
        if (row < m_selectedItems.size()) {
            m_selectedItems.removeAt(row);
            m_selectedProductsTable->removeRow(row);
            updateTotalAmount();

            // Reset selection if removed
            if (m_currentSelectedRow == row) {
                m_currentSelectedRow = -1;
            } else if (m_currentSelectedRow > row) {
                m_currentSelectedRow--;
            }
        }
    });

    m_selectedProductsTable->setCellWidget(row, 4, removeBtn);

    // Add to visual list (for UI design)
    QWidget *productWidget = new QWidget(this);
    productWidget->setObjectName(QString("selected_%1").arg(item.productId));
    productWidget->setStyleSheet("background-color: #1e1e1e; color: white; border-radius: 5px; margin: 2px;");

    QHBoxLayout *productLayout = new QHBoxLayout(productWidget);
    productLayout->setContentsMargins(10, 5, 10, 5);

    QVBoxLayout *infoLayout = new QVBoxLayout();
    QLabel *nameLabel = new QLabel(item.productName, productWidget);
    nameLabel->setStyleSheet("font-size: 14px; font-weight: bold;");
    QLabel *unitLabel = new QLabel(QString("%1 Units at %2 Rs. / Unit").arg(item.quantity).arg(item.unitPrice, 0, 'f', 2), productWidget);
    unitLabel->setStyleSheet("font-size: 12px; color: #aaa;");

    infoLayout->addWidget(nameLabel);
    infoLayout->addWidget(unitLabel);

    QLabel *priceLabel = new QLabel(QString("%1 Rs.").arg(item.totalPrice, 0, 'f', 2), productWidget);
    priceLabel->setStyleSheet("font-size: 14px; font-weight: bold;");
    priceLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QPushButton *removeButton = new QPushButton("×", productWidget);
    removeButton->setStyleSheet("background-color: transparent; color: white; font-size: 16px; font-weight: bold; border: none;");
    removeButton->setCursor(Qt::PointingHandCursor);
    connect(removeButton, &QPushButton::clicked, [this, row]() {
        if (row < m_selectedItems.size()) {
            m_selectedItems.removeAt(row);
            m_selectedProductsTable->removeRow(row);
            updateTotalAmount();

            // Update visual list
            refreshProductList();
        }
    });

    productLayout->addLayout(infoLayout);
    productLayout->addStretch();
    productLayout->addWidget(priceLabel);
    productLayout->addWidget(removeButton);

    // Store row index for reference
    productWidget->setProperty("rowIndex", row);

    m_selectedLayout->addWidget(productWidget);

    // Update total amount
    updateTotalAmount();

    // Select the newly added item
    highlightSelectedProduct(row);
}

void SalesDashboard::updateSelectedItemQuantity(int index, bool increase)
{
    if (index < 0 || index >= m_selectedItems.size()) {
        return;
    }

    // Update quantity
    if (increase) {
        // Check if we have enough stock
        if (m_selectedItems[index].quantity < m_selectedItems[index].available) {
            m_selectedItems[index].quantity++;
        } else {
            QMessageBox::warning(this, "Insufficient Stock",
                                 "Cannot add more of this product. Stock limit reached.");
            return;
        }
    } else {
        if (m_selectedItems[index].quantity > 1) {
            m_selectedItems[index].quantity--;
        } else {
            return; // Don't reduce below 1
        }
    }

    // Update total price for this item
    m_selectedItems[index].totalPrice = m_selectedItems[index].quantity * m_selectedItems[index].unitPrice;

    // Update table
    m_selectedProductsTable->item(index, 2)->setText(QString::number(m_selectedItems[index].quantity));
    m_selectedProductsTable->item(index, 3)->setText(QString::number(m_selectedItems[index].totalPrice, 'f', 2));

    // Update total amount
    updateTotalAmount();
}

void SalesDashboard::updateTotalAmount()
{
    m_totalAmount = 0.0;
    for (const SaleItem &item : m_selectedItems) {
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

void SalesDashboard::onRemoveSelectedProduct()
{
    if (m_currentSelectedRow >= 0 && m_currentSelectedRow < m_selectedItems.size()) {
        m_selectedItems.removeAt(m_currentSelectedRow);
        m_selectedProductsTable->removeRow(m_currentSelectedRow);
        updateTotalAmount();
        m_currentSelectedRow = -1;
    }
}

void SalesDashboard::onSellProductsClicked()
{
    if (m_selectedItems.isEmpty()) {
        QMessageBox::warning(this, "No Products Selected", "Please select at least one product to complete the sale.");
        return;
    }

    // Get current user ID
    int userId = m_authManager->getCurrentUserId();
    if (userId <= 0) {
        QMessageBox::critical(this, "Authentication Error", "User information not available. Please log in again.");
        return;
    }

    // Process the sale
    if (m_salesManager->processSale(m_selectedItems, userId)) {
        QMessageBox::information(this, "Sale Completed",
                                 QString("Sale of %1 items totaling %2 Rs. has been successfully recorded.")
                                     .arg(m_selectedItems.size())
                                     .arg(m_totalAmount, 0, 'f', 2));

        // Reset the sales area
        resetSalesArea();

        // Refresh the sales table
        refreshSalesTable();
    } else {
        QMessageBox::critical(this, "Sale Failed", "There was an error processing the sale. Please try again.");
    }
}

void SalesDashboard::onClearSelectionClicked()
{
    // Ask for confirmation
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Clear Selection",
                                                              "Are you sure you want to clear all selected products?",
                                                              QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        resetSalesArea();
    }
}
