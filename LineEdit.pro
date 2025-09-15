QT       += core gui widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    batchtab.cpp \
    datastructures.cpp \
    dattablemodel.cpp \
    main.cpp \
    mainwindow.cpp \
    plotwidget.cpp \
    previewdialog.cpp \
    projectmanager.cpp \
    projectmodel.cpp \
    projecttreeview.cpp \
    tablemodel.cpp

HEADERS += \
    batchtab.h \
    datastructures.h \
    dattablemodel.h \
    mainwindow.h \
    plotwidget.h \
    previewdialog.h \
    projectmanager.h \
    projectmodel.h \
    projecttreeview.h \
    tablemodel.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

msvc{
    QMAKE_CFLAGS += /utf-8
    QMAKE_CXXFLAGS += /utf-8
}
