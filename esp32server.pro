QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets serialport network

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    centralwidget.cpp \
    connection.cpp \
    main.cpp \
    mainwindow.cpp \
    serialcomm.cpp \
    server.cpp

HEADERS += \
    centralwidget.hpp \
    connection.hpp \
    esp32commands.hpp \
    mainwindow.hpp \
    serialcomm.hpp \
    server.hpp \
    servercommands.hpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
