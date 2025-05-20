QT       += core gui sql
QT += qml
QT       += core gui charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    debtmanager.cpp \
    main.cpp \
    mainwindow.cpp \
    databasehandler.cpp \
    authmanager.cpp \
    productmanager.cpp \
    salesdashboard.cpp \
    salesmanager.cpp \
    stockmanager.cpp \
    vendormanager.cpp \
    workermanager.cpp

HEADERS += \
    clickableWidget.h \
    debtmanager.h \
    mainwindow.h \
    databasehandler.h \
    authmanager.h \
    productmanager.h \
    salesdashboard.h \
    salesmanager.h \
    stockmanager.h \
    vendormanager.h \
    workermanager.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    img.qrc
