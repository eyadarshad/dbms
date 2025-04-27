#include "mainwindow.h"
#include <QApplication>
#include <QPushButton>
#include <QStringList>
#include <QJSEngine>  // For evaluating math expressions

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}

