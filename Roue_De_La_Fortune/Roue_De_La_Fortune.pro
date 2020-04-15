QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    config/config.cpp \
    entity/localplayer.cpp \
    entity/player.cpp \
    event/eventmanager.cpp \
    ui/game.cpp \
    ui/inscription.cpp \
    ui/servernavigator.cpp \
    main.cpp \
    network/tcpclient.cpp \    
    protocol/protocolhandler.cpp \        
    ui/connection.cpp \    
    main.cpp \

HEADERS += \
    config/config.h \
    entity/localplayer.h \
    entity/player.h \
    event/eventmanager.h \
    ui/game.h \
    ui/inscription.h \
    ui/servernavigator.h \
    network/tcpclient.h \
    protocol/protocolhandler.h \
    ui/connection.h \

FORMS += \
    ui/connection.ui \
    ui/game.ui \
    ui/inscription.ui \
    ui/servernavigator.ui \

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
