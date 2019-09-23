#-------------------------------------------------
#
# Project created by QtCreator 2019-09-02T11:05:58
#
#-------------------------------------------------

QT       += core gui xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = resources_scheduler
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
    src/main.cpp \
    src/models/tablemodel.cpp \
    src/utils/frameless_helper.cpp \
    src/views/mainwindow.cpp \
    src/views/titlebar.cpp \
    src/views/widget.cpp

HEADERS += \
    src/models/elements.h \
    src/models/tablemodel.h \
    src/utils/frameless_helper.h \
    src/utils/macro.h \
    src/views/mainwindow.h \
    src/views/titlebar.h \
    src/views/widget.h

INCLUDEPATH += qgis

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    qdarkstyle/style.qrc

# 第三方库加载
QGIS_3RD_PARTY = $$PWD/3rdparty
QGIS_LIB_PATH = $${QGIS_3RD_PARTY}/qgis3.4.9/lib

INCLUDEPATH += $${QGIS_3RD_PARTY}
INCLUDEPATH += $${QGIS_3RD_PARTY}/qgis3.4.9/include

CONFIG(debug,debug|release){
    LIBS += $${QGIS_LIB_PATH}/Debug/gdal_i.lib
    LIBS += $${QGIS_LIB_PATH}/Debug/qgis_analysis.lib
    LIBS += $${QGIS_LIB_PATH}/Debug/qgis_app.lib
    LIBS += $${QGIS_LIB_PATH}/Debug/qgis_core.lib
    LIBS += $${QGIS_LIB_PATH}/Debug/qgis_customwidgets.lib
    LIBS += $${QGIS_LIB_PATH}/Debug/qgis_gui.lib
    LIBS += $${QGIS_LIB_PATH}/Debug/qgis_native.lib
}else{
    LIBS += $${QGIS_LIB_PATH}/Release/qgis_analysis.lib
    LIBS += $${QGIS_LIB_PATH}/Release/qgis_app.lib
    LIBS += $${QGIS_LIB_PATH}/Release/qgis_core.lib
    LIBS += $${QGIS_LIB_PATH}/Release/qgis_customwidgets.lib
    LIBS += $${QGIS_LIB_PATH}/Release/qgis_gui.lib
    LIBS += $${QGIS_LIB_PATH}/Release/qgis_native.lib
}
