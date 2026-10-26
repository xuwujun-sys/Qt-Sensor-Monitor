QT       += core gui charts network sql serialport serialbus

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    canmanger.cpp \
    serialmanger.cpp \
    tcpmanger.cpp \
    sensorchart.cpp \
    databasemanger.cpp \
    historywidget.cpp \
    form.cpp \
    main.cpp \
    mainwindow.cpp \
    tcpphraser.cpp

HEADERS += \
    canmanger.h \
    serialmanger.h \
    tcpmanger.h \
    sensorchart.h \
    databasemanger.h \
    historywidget.h \
    form.h \
    mainwindow.h \
    sensordata.h \
    tcpphraser.h

FORMS += \
    historywidget.ui \
    form.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc
