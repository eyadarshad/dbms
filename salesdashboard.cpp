#include "salesdashboard.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDebug>
#include <QDateTime>

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
{
    setupUI();

    // Connect signals and slots
    connect(m_productSearchEdit, &QLineEdit::textChanged, this, &SalesDashboard::onProductSearchTextChanged);
    connect(m_salesSearchEdit, &QLineEdit::textChanged, this, &SalesDashboard::onSalesSearchTextChanged);
    connect(m_productsTable, &QTableWidget::cellClicked, this, &SalesDashboard::onProductSelected);
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
}

void SalesDashboard::setupUI()
{
    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Left and right panels
    QHBoxLayout *hLayout = new QHBoxLayout();
    QVBoxLayout *leftLayout = new QVBoxLayout();
    QVBoxLayout *rightLayout = new QVBoxLayout();

    // ---- Left panel ----
    // Products search
    setupProductSearch();
    leftLayout->addWidget(m_productSearchEdit);

    // Products table
    setupProductsTable();
    leftLayout->addWidget(m_productsTable);

    // Sales search
    m_salesSearchEdit = new QLineEdit(this);
    m_salesSearchEdit->setPlaceholderText("Search Sales...");
    leftLayout->addWidget(m_salesSearchEdit);

    // Sales table
    setupSalesTable();
    leftLayout->addWidget(m_salesTable);

    // ---- Right panel ----
    // Selected products list
    setupSelectedProductsTable();
    rightLayout->addWidget(m_selectedProductsTable);

    // Total section
    QHBoxLayout *totalLayout = new QHBoxLayout();
    m_totalLabel = new QLabel("Total: 0.00 Rs.", this);
    m_totalLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_totalLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    totalLayout->addWidget(m_totalLabel);
    rightLayout->addLayout(totalLayout);

    // Action buttons
    setupActionButtons();
    QHBoxLayout *actionLayout = new QHBoxLayout();
    actionLayout->addWidget(m_clearSelectionBtn);
    actionLayout->addWidget(m_sellProductsBtn);
    rightLayout->addLayout(actionLayout);

    // Add left and right panels to the main layout
    hLayout->addLayout(leftLayout, 3);
    hLayout->addLayout(rightLayout, 2);
    mainLayout->addLayout(hLayout);

    setLayout(mainLayout);
}

void SalesDashboard::setupProductSearch()
{
    m_productSearchEdit = new QLineEdit(this);
    m_productSearchEdit->setPlaceholderText("Search Products...");
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
}

void SalesDashboard::setupSelectedProductsTable()
{
    m_selectedProductsTable = new QTableWidget(this);
    m_selectedProductsTable->setColumnCount(5);
    m_selectedProductsTable->setHorizontalHeaderLabels({"Product", "Price", "Qty", "Total", "Actions"});
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
        "Sales ID", "Salesman ID", "Product ID", "Product Name",
        "Price", "Category", "Quantity", "Date/Time"
    });
    m_salesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_salesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_salesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void SalesDashboard::setupActionButtons()
{
    m_sellProductsBtn = new QPushButton("Sell Products", this);
    m_sellProductsBtn->setStyleSheet("background-color: #4287f5; color: white; padding: 8px;");

    m_clearSelectionBtn = new QPushButton("Clear Selection", this);
    m_clearSelectionBtn->setStyleSheet("background-color: #f54242; color: white; padding: 8px;");

    m_addQtyBtn = new QPushButton("+", this);
    m_removeQtyBtn = new QPushButton("-", this);
}

void SalesDashboard::refreshProductList()
{
    m_productManager->loadProducts(m_productsTable);
}

void SalesDashboard::refreshSelectedProducts()
{
    m_selectedProductsTable->setRowCount(0);

    int row = 0;
    for (const SaleItem &item : m_selectedItems) {
        m_selectedProductsTable->insertRow(row);

        m_selectedProductsTable->setItem(row, 0, new QTableWidgetItem(item.productName));
        m_selectedProductsTable->setItem(row, 1, new QTableWidgetItem(QString::number(item.unitPrice, 'f', 2) + " Rs."));
        m_selectedProductsTable->setItem(row, 2, new QTableWidgetItem(QString::number(item.quantity)));
        m_selectedProductsTable->setItem(row, 3, new QTableWidgetItem(QString::number(item.totalPrice, 'f', 2) + " Rs."));

        QPushButton *removeBtn = new QPushButton("X", this);
        removeBtn->setStyleSheet("background-color: #f54242; color: white;");
        connect(removeBtn, &QPushButton::clicked, this, [this, row]() {
            m_selectedItems.removeAt(row);
            refreshSelectedProducts();
            updateTotals();
        });

        m_selectedProductsTable->setCellWidget(row, 4, removeBtn);
        row++;
    }

    updateTotals();
}

void SalesDashboard::refreshSalesTable()
{
    m_salesManager->loadSales(m_salesTable);
}

void SalesDashboard::onProductSearchTextChanged(const QString &text)
{
    if (text.isEmpty()) {
        refreshProductList();
    } else {
        m_productManager->searchProducts(m_productsTable, text);
    }
}

void SalesDashboard::onSalesSearchTextChanged(const QString &text)
{
    if (text.isEmpty()) {
        refreshSalesTable();
    } else {
        m_salesManager->searchSales(m_salesTable, text);
    }
}

void SalesDashboard::onProductSelected(int row, int column)
{
    Q_UNUSED(column)

    int productId = m_productsTable->item(row, 0)->text().toInt();
    QString productName = m_productsTable->item(row, 1)->text();
    double price = m_productsTable->item(row, 2)->text().toDouble();
    int stockAvailable = m_productsTable->item(row, 4)->text().toInt();

    // Check if already in the list
    for (int i = 0; i < m_selectedItems.size(); i++) {
        if (m_selectedItems[i].productId == productId) {
            // Already selected, increment quantity
            if (m_selectedItems[i].quantity < stockAvailable) {
                m_selectedItems[i].quantity++;
                m_selectedItems[i].totalPrice = m_selectedItems[i].quantity * m_selectedItems[i].unitPrice;
                refreshSelectedProducts();
            } else {
                QMessageBox::warning(this, "Stock Limit", "No more stock available for this product!");
            }
            return;
        }
    }

    // Add new selected product
    if (stockAvailable > 0) {
        SaleItem item;
        item.productId = productId;
        item.productName = productName;
        item.unitPrice = price;
        item.quantity = 1;
        item.totalPrice = price;

        m_selectedItems.append(item);
        refreshSelectedProducts();
    } else {
        QMessageBox::warning(this, "Out of Stock", "This product is out of stock!");
    }
}

void SalesDashboard::onProductRemoved()
{
    // This is handled by the button in refreshSelectedProducts()
}

void SalesDashboard::onQuantityChanged(bool increase)
{
    // Get selected row
    QModelIndexList indexes = m_selectedProductsTable->selectionModel()->selectedRows();
    if (indexes.isEmpty())
        return;

    int row = indexes.first().row();
    if (row < 0 || row >= m_selectedItems.size())
        return;

    // Get product ID to check stock
    int productId = m_selectedItems[row].productId;

    // Find available stock
    int availableStock = 0;
    for (int i = 0; i < m_productsTable->rowCount(); i++) {
        if (m_productsTable->item(i, 0)->text().toInt() == productId) {
            availableStock = m_productsTable->item(i, 4)->text().toInt();
            break;
        }
    }

    // Update quantity
    if (increase) {
        if (m_selectedItems[row].quantity < availableStock) {
            m_selectedItems[row].quantity++;
        } else {
            QMessageBox::warning(this, "Stock Limit", "No more stock available for this product!");
            return;
        }
    } else {
        if (m_selectedItems[row].quantity > 1) {
            m_selectedItems[row].quantity--;
        } else {
            // Option to remove product completely
            if (QMessageBox::question(this, "Remove Product",
                                      "Quantity will be zero. Remove product from selection?",
                                      QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
                m_selectedItems.removeAt(row);
            }
        }
    }

    // Update total price
    if (row < m_selectedItems.size()) {
        m_selectedItems[row].totalPrice = m_selectedItems[row].quantity * m_selectedItems[row].unitPrice;
    }

    refreshSelectedProducts();
}

void SalesDashboard::onSellProductsClicked()
{
    if (m_selectedItems.isEmpty()) {
        QMessageBox::warning(this, "No Products", "No products selected for sale!");
        return;
    }

    // Confirm sale
    if (QMessageBox::question(this, "Confirm Sale",
                              QString("Process sale for %1 products with total amount %2 Rs.?")
                                  .arg(m_selectedItems.size())
                                  .arg(m_totalAmount),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    // Process the sale - pass current user ID and optional debtor ID (if needed)
    int userId = m_authManager->getCurrentUserId();

    // Process the sale (debtorId set to -1 for cash sale)
    if (m_salesManager->processSale(m_selectedItems, userId)) {
        QMessageBox::information(this, "Success", "Sale processed successfully!");

        // Clear selection
        m_selectedItems.clear();
        refreshSelectedProducts();

        // Refresh product list and sales table
        refreshProductList();
        refreshSalesTable();
    } else {
        QMessageBox::critical(this, "Error", "Failed to process sale. Please try again.");
    }
}

void SalesDashboard::onClearSelectionClicked()
{
    if (!m_selectedItems.isEmpty()) {
        if (QMessageBox::question(this, "Clear Selection",
                                  "Are you sure you want to clear all selected products?",
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            m_selectedItems.clear();
            refreshSelectedProducts();
        }
    }
}

void SalesDashboard::updateTotals()
{
    m_totalAmount = 0.0;

    for (const SaleItem &item : m_selectedItems) {
        m_totalAmount += item.totalPrice;
    }

    m_totalLabel->setText(QString("Total: %1 Rs.").arg(m_totalAmount, 0, 'f', 2));
}
