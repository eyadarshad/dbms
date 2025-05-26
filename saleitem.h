#ifndef SALEITEM_H
#define SALEITEM_H

#include <QString>

struct SaleItem {
    int productId;
    QString productName;
    double unitPrice;
    QString category;
    int available;
    int quantity;
    double totalPrice;
};

#endif // SALEITEM_H
