TARGET = Mis2x265
CONFIG += qt
CONFIG += precompile_header
!win32:CONFIG += console

QT += gui \
    core
UI_DIR = uiHeaders
TEMPLATE = app
INCLUDEPATH += .

# Qt 5+ adjustments
greaterThan(QT_MAJOR_VERSION, 4) { # QT5+
    QT += widgets # for all widgets
    win32-msvc*:DEFINES += NOMINMAX
}

win32 {
  DEFINES += BUILDVERSION=\\\"$$system('echo %date:~6,4%.%date:~3,2%.%date:~0,2%').1\\\"
  
 } else: {
  DEFINES += BUILDVERSION=\\\"$$system(date '+%y%m.%d.1')\\\"
  macx:DEFINES += MACBUILDDATE=\\\"$$system(date '+%y%m.%d.1')\\\"
} 

# OS specific adjustments
win32-msvc* { 
    message(Building for Windows using Qt $$QT_VERSION)
    CONFIG += c++11 # C++11 support
    QMAKE_CXXFLAGS += /bigobj # allow big objects
    !contains(QMAKE_HOST.arch, x86_64):QMAKE_LFLAGS += /LARGEADDRESSAWARE # allow the use more of than 2GB of RAM on 32bit Windows
    
    # # add during static build
    # QMAKE_CFLAGS_RELEASE += -MT
    # QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO += -MT
    # QMAKE_CFLAGS_DEBUG = -Zi -MTd
    # QMAKE_LFLAGS += /DYNAMICBASE:NO
    # for Windows XP compatibility
    contains(QMAKE_HOST.arch, x86_64):QMAKE_LFLAGS += /SUBSYSTEM:WINDOWS,5.02 # Windows XP 64bit
    else:QMAKE_LFLAGS += /SUBSYSTEM:WINDOWS,5.01 # Windows XP 32bit
}
else:macx { 
    message(Building for Mac OS X using Qt $$QT_VERSION)
    LIBS += -framework \
        Cocoa
    QMAKE_CFLAGS_RELEASE += -fvisibility=hidden
    QMAKE_CXXFLAGS += -std=c++0x
    QMAKE_CXXFLAGS_RELEASE += -fvisibility=hidden \
        -fvisibility-inlines-hidden
       QMAKE_INFO_PLIST = resources/Info.plist
}
else:linux-* { 
    message(Building for Linux using Qt $$QT_VERSION)
    QT += dbus # dbus support
    QMAKE_CXXFLAGS += -std=c++0x
}

HEADERS   += Mis2x265.h
SOURCES   += main.cpp \
    Mis2x265.cpp
FORMS     += Mis2x265.ui    
