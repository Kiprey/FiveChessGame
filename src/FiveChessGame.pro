#-------------------------------------------------
#
# Project created by QtCreator
#
#-------------------------------------------------
#
# E-Mail: Hipre@qq.com
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FiveChessGame
TEMPLATE = app


SOURCES += main.cpp\
    MsgTextEdit/MsgTextEdit.cpp \
    NetworkModule/NetwortModule.cpp \
    OnlineOptionWidget/OnlineOptionWidget.cpp \
    Widget/RawWidget.cpp \
	Widget/Widget1.cpp \
    Widget/Widget2.cpp \
    Widget/AIPutChess.cpp

HEADERS  +=\
    MsgEnums.h \
    MsgTextEdit/MsgTextEdit.h \
    Widget/Widget.h \
    NetworkModule/NetworkModule.h \
    OnlineOptionWidget/OnlineOptionWidget.h

FORMS    += widget.ui

RC_FILE = FiveChessGame.rc

CONFIG += mobility
MOBILITY = 

