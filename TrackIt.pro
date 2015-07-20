#-------------------------------------------------
#
# Project created by QtCreator 2012-08-23T20:27:06
#
#-------------------------------------------------

QT       += core gui opengl xml

TARGET = TrackIt

TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    videowidget.cpp \
    datawidget.cpp \
    types.cpp \
    category.cpp \
    object.cpp \
    bboxdelegate.cpp \
    scrollarea.cpp \
    idcounter.cpp \
    julia.cpp

HEADERS  += mainwindow.h \
    videowidget.h \
    datawidget.h \
    types.h \
    category.h \
    object.h \
    bboxdelegate.h \
    scrollarea.h \
    idcounter.h \
    julia.h

win32:{LIBS += -lopencv_highgui242 \
               -lopencv_core242}
unix:LIBS += "-L/usr/local/lib" -lopencv_core -lopencv_highgui

unix:{INCLUDEPATH += /usr/local/include/opencv2/
      INCLUDEPATH += /usr/local/include/opencv2/core}

RESOURCES += icons.qrc

OTHER_FILES += \
    ReadMe.txt

win32:{QMAKE_LFLAGS = -enable-auto-import
       RC_FILE = icon.rc}
