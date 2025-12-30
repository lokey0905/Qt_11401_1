QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Core/BaseComponent.cpp \
    ControlPanel.cpp \
    Core/SettingsManager.cpp \
    ToolSettingsForm.cpp \
    Widgets/TimeWidget.cpp \
    Widgets/CpuWidget.cpp \
    Widgets/DiskWidget.cpp \
    main.cpp

HEADERS += \
    Core/BaseComponent.h \
    ControlPanel.h \
    Core/SettingsManager.h \
    ToolSettingsForm.h \
    Widgets/TimeWidget.h \
    Widgets/CpuWidget.h \
    Widgets/DiskWidget.h

FORMS += \
    ControlPanel.ui \
    ToolSettingsForm.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

INCLUDEPATH += Core Widgets

win32: LIBS += -lpdh -lPowrProf
