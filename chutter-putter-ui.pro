HEADERS += \
    PeerSession.h \
    messagetab.h \
    networkbackend.h \
    peerlist.h \
    ui.h

SOURCES += \
    PeerSession.cpp \
    main.cpp \
    messagetab.cpp \
    networkbackend.cpp \
    peerlist.cpp \
    ui.cpp

QT += widgets

LIBS += -lboost_system -lpthread -lboost_json
