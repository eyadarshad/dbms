#include "mainwindow.h"
#include <QApplication>
#include <QPushButton>
#include <QStringList>
#include <QJSEngine>  // For evaluating math expressions

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon("C:/Users/EYAD/Documents/dbms-har/dbms-har/images/icon.png"));
    MainWindow w;
    w.setWindowTitle("  UtiliSOFT");
    w.show();

    return app.exec();
}

