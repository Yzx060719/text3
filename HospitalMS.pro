QT += core gui widgets sql

CONFIG += c++17
CONFIG -= app_bundle
TEMPLATE = app
TARGET = HospitalMS

# 源码为 UTF-8，保证中文字符串在编译期不被转换
QMAKE_CXXFLAGS += -finput-charset=UTF-8 -fexec-charset=UTF-8
msvc: QMAKE_CXXFLAGS += /utf-8

INCLUDEPATH += src

HEADERS += \
    src/dbmanager.h \
    src/delegates.h \
    src/homepage.h \
    src/logpage.h \
    src/loginpage.h \
    src/mainwindow.h \
    src/managepage.h \
    src/patientdialog.h \
    src/patientpage.h \
    src/statisticsdialog.h

SOURCES += \
    src/main.cpp \
    src/dbmanager.cpp \
    src/delegates.cpp \
    src/homepage.cpp \
    src/logpage.cpp \
    src/loginpage.cpp \
    src/mainwindow.cpp \
    src/managepage.cpp \
    src/patientdialog.cpp \
    src/patientpage.cpp \
    src/statisticsdialog.cpp

RESOURCES += resources.qrc
