#ifndef CLICKABLEWIDGET_H
#define CLICKABLEWIDGET_H

#include <QWidget>
#include <QMouseEvent>

class ClickableWidget : public QWidget {
    Q_OBJECT
public:
    explicit ClickableWidget(QWidget *parent = nullptr) : QWidget(parent) {}

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent *event) override {
        emit clicked();
        QWidget::mousePressEvent(event); // optional if you want default behavior
    }
};

#endif // CLICKABLEWIDGET_H
