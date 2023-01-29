#  *********************************************************************************************************************************************************
#  @file     :ESPlot-Computer Software.pro
#  @brief    :Project file of the QT project of ESPlot
#  *********************************************************************************************************************************************************
#  ESPlot allows real-time communication between an embedded system and a computer offering signal processing and plotting capabilities and it relies on
#  hardware graphics acceleration for systems disposing of OpenGL-compatible graphic units. More info at www.uni-saarland.de/lehrstuhl/nienhaus/esplot.

#  Copyright (C) Universität des Saarlandes 2020. Authors: Emanuele Grasso and Niklas König.

#  The Software and the associated materials have been developed at the Universität des Saarlandes (hereinafter "UdS").
#  Any copyright or patent right is owned by and proprietary material of the UdS hereinafter the “Licensor”.

#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU Affero General Public License as
#  published by the Free Software Foundation, either version 3 of the
#  License, or any later version.

#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#  GNU Affero General Public License for more details.

#  You should have received a copy of the GNU Affero General Public License
#  along with this program. If not, see <https://www.gnu.org/licenses/>.

#  This Agreement shall be governed by the laws of the Federal Republic of Germany except for the UN Sales Convention and the German rules of conflict of law.

#  Commercial licensing opportunities
#  For commercial uses of the Software beyond the conditions applied by AGPL 3.0 please contact the Licensor sending an email to patentverwertungsagentur@uni-saarland.de
#  *********************************************************************************************************************************************************

TARGET = ESPlot

QT += widgets
QT += core
QT += serialport

CONFIG += c++11
CONFIG += qt

QMAKE_CFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CFLAGS_RELEASE += -O3
QMAKE_CXXFLAGS_RELEASE += -O3

RESOURCES = resources.qrc

RC_ICONS =  Icons/esp_logo_icon.ico

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += USE_VERTEX_ID

SOURCES += \
    3rdparty/kissFFT/kiss_fft.c \
    3rdparty/kissFFT/kiss_fftr.c \
    Managers/filenamegenerator.cpp \
    main.cpp \
    mainApplication.cpp \
    LogBrowser/logbrowser.cpp \
    LogBrowser/logbrowserdialog.cpp \
    CommProtocol/comm_prot.cpp \
    CommProtocol/ft4222_dev.cpp \
    CommProtocol/serial_dev.cpp \
    FontManager/fontmanager.cpp \
    FontManager/fontpreview.cpp \
    FontManager/glyphloader.cpp \
    Creators/grid.cpp \
    Creators/legendCreator.cpp \
    Creators/statcreator.cpp \
    Creators/textrenderer.cpp \
    Creators/tooltipcreator.cpp \
    Dialogs/deveditor.cpp \
    Dialogs/fft_plot_window.cpp \
    Dialogs/glwindow.cpp \
    Dialogs/infodialog.cpp \
    Dialogs/plot_window.cpp \
    Dialogs/prefDlg.cpp \
    Dialogs/sigassdlg.cpp \
    Dialogs/xy_plot_window.cpp \
    Dialogs/connectdlg.cpp \
    Managers/fftmanager.cpp \
    Managers/matlabfilesaver.cpp \
    Managers/prefmanager.cpp \
    Managers/sgnalplottermanager.cpp \
    Managers/signal_data.cpp \
    Creators/grid_xy.cpp \
    Dialogs/xy_glwindow.cpp

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =
# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    3rdparty/kissFFT/_kiss_fft_guts.h \
    3rdparty/kissFFT/kiss_fft.h \
    3rdparty/kissFFT/kiss_fftr.h \
    Managers/filenamegenerator.h \
    Creators/tooltipcreator.h \
    FontManager/fontmanager.h \
    definitions.h \
    FontManager/glyphloader.h \
    Creators/grid.h \
    Dialogs/infodialog.h \
    Creators/legendCreator.h \
    mainApplication.h \
    Managers/matlabfilesaver.h \
    Dialogs/prefDlg.h \
    Managers/preferences.h \
    Managers/prefmanager.h \
    Managers/sgnalplottermanager.h \
    Dialogs/sigassdlg.h \
    Managers/signal_data.h \
    Creators/statcreator.h \
    LogBrowser/logbrowser.h \
    LogBrowser/logbrowserdialog.h \
    CommProtocol/comm_dev.h \
    CommProtocol/comm_prot.h \
    CommProtocol/ft4222_dev.h \
    CommProtocol/serial_dev.h \
    FontManager/fontpreview.h \
    FontManager/glyphloader.h \
    Creators/grid.h \
    Creators/legendCreator.h \
    Creators/statcreator.h \
    Creators/textrenderer.h \
    Creators/tooltipcreator.h \
    Dialogs/deveditor.h \
    Dialogs/fft_plot_window.h \
    Dialogs/glwindow.h \
    Dialogs/infodialog.h \
    Dialogs/plot_window.h \
    Dialogs/xy_plot_window.h \
    Dialogs/connectdlg.h \
    Managers/fftmanager.h \
    Managers/matlabfilesaver.h \
    Managers/preferences.h \
    Managers/prefmanager.h \
    Managers/signal_data.h \
    Creators/grid_xy.h \
    Dialogs/xy_glwindow.h

#CONFIG += \
#    hide_symbols \
#    rtti_off warn_off \
#    installed

MODULE_INCLUDEPATH += $$PWD/include

INCLUDEPATH += 3rdparty/FreeType/include
INCLUDEPATH += 3rdparty/FreeType/include/freetype/config

SOURCES += \
    3rdparty/FreeType/src/autofit/afangles.c \
    3rdparty/FreeType/src/autofit/afdummy.c \
    3rdparty/FreeType/src/autofit/afglobal.c \
    3rdparty/FreeType/src/autofit/afhints.c \
    3rdparty/FreeType/src/autofit/aflatin.c \
    3rdparty/FreeType/src/autofit/afloader.c \
    3rdparty/FreeType/src/autofit/afmodule.c \
    3rdparty/FreeType/src/autofit/autofit.c \
    3rdparty/FreeType/src/base/ftbase.c \
    3rdparty/FreeType/src/base/ftbitmap.c \
    3rdparty/FreeType/src/base/ftbbox.c \
    3rdparty/FreeType/src/base/ftdebug.c \
    3rdparty/FreeType/src/base/ftglyph.c \
    3rdparty/FreeType/src/base/ftfntfmt.c \
    3rdparty/FreeType/src/base/ftinit.c \
    3rdparty/FreeType/src/base/ftlcdfil.c \
    3rdparty/FreeType/src/base/ftmm.c \
    3rdparty/FreeType/src/base/ftsynth.c \
    3rdparty/FreeType/src/base/fttype1.c \
    3rdparty/FreeType/src/bdf/bdf.c \
    3rdparty/FreeType/src/cache/ftcache.c \
    3rdparty/FreeType/src/cff/cff.c \
    3rdparty/FreeType/src/cid/type1cid.c \
    3rdparty/FreeType/src/gzip/ftgzip.c \
    3rdparty/FreeType/src/lzw/ftlzw.c \
    3rdparty/FreeType/src/otvalid/otvalid.c \
    3rdparty/FreeType/src/otvalid/otvbase.c \
    3rdparty/FreeType/src/otvalid/otvcommn.c \
    3rdparty/FreeType/src/otvalid/otvgdef.c \
    3rdparty/FreeType/src/otvalid/otvgpos.c \
    3rdparty/FreeType/src/otvalid/otvgsub.c \
    3rdparty/FreeType/src/otvalid/otvjstf.c \
    3rdparty/FreeType/src/otvalid/otvmod.c \
    3rdparty/FreeType/src/pcf/pcf.c \
    3rdparty/FreeType/src/pfr/pfr.c \
    3rdparty/FreeType/src/psaux/psaux.c \
    3rdparty/FreeType/src/pshinter/pshinter.c \
    3rdparty/FreeType/src/psnames/psmodule.c \
    3rdparty/FreeType/src/raster/raster.c \
    3rdparty/FreeType/src/sfnt/sfnt.c \
    3rdparty/FreeType/src/smooth/smooth.c \
    3rdparty/FreeType/src/truetype/truetype.c \
    3rdparty/FreeType/src/type1/type1.c \
    3rdparty/FreeType/src/type42/type42.c \
    3rdparty/FreeType/src/winfonts/winfnt.c

win32 {
    SOURCES += 3rdparty/FreeType/src/base/ftsystem.c
} else {
   INCLUDEPATH += 3rdparty/FreeType/builds/unix
   SOURCES += 3rdparty/FreeType/builds/unix/ftsystem.c
}

DEFINES += FT2_BUILD_LIBRARY

win32 {

INCLUDEPATH += $$PWD/3rdparty/ftdi/ftd2xx
DEPENDPATH += $$PWD/3rdparty/ftdi/ftd2xx
INCLUDEPATH += $$PWD/3rdparty/ftdi/LibFT4222/inc
DEPENDPATH += $$PWD/3rdparty/ftdi/LibFT4222/inc

    contains(QT_ARCH, i386) {
        error("Windows 32-bit builds are not officially supported")
    } else {
        message("Windows 64-bit build")
        LIBS += -L$$PWD/3rdparty/ftdi/ftd2xx/amd64/ -lftd2xx
        PRE_TARGETDEPS += $$PWD/3rdparty/ftdi/ftd2xx/amd64/ftd2xx.lib
        LIBS += -L$$PWD/3rdparty/ftdi/LibFT4222/lib/amd64/ -lLibFT4222
        PRE_TARGETDEPS += $$PWD/3rdparty/ftdi/LibFT4222/lib/amd64/LibFT4222.lib
    }
}

linux {

message("Linux build")
DEFINES += NO_RANDOM_COLOR
INCLUDEPATH += $$PWD/3rdparty/ftdi/ftd2xx
DEPENDPATH += $$PWD/3rdparty/ftdi/ftd2xx
INCLUDEPATH += $$PWD/3rdparty/ftdi/LibFT4222/inc
DEPENDPATH += $$PWD/3rdparty/ftdi/LibFT4222/inc

LIBS += -lft4222 -Wl,-rpath,/usr/local/lib
}

macos {

message("MacOS build")
INCLUDEPATH += $$PWD/3rdparty/ftdi/ftd2xx
DEPENDPATH += $$PWD/3rdparty/ftdi/ftd2xx
INCLUDEPATH += $$PWD/3rdparty/ftdi/LibFT4222/inc
DEPENDPATH += $$PWD/3rdparty/ftdi/LibFT4222/inc

LIBS += -L$$PWD/3rdparty/ftdi/ -lft4222.1.4.2.184
}

DISTFILES += \
    Icons/Add.ico \
    Icons/AddXY.ico \
    Icons/AddXYZ.ico \
    Icons/Associate.ico \
    Icons/Autorecord.ico \
    Icons/Clear.ico \
    Icons/Connect Disconnect Icon License \
    Icons/Connect.ico \
    Icons/Deassociate.ico \
    Icons/Disconnect.ico \
    Icons/EsPlotLogo.png \
    Icons/LATlogo.jpg \
    Icons/Pause.ico \
    Icons/Play.ico \
    Icons/Record.ico \
    Icons/Remove.ico \
    Icons/RemoveXY.ico \
    Icons/RemoveXYZ.ico \
    Icons/SUlogo.jpg \
    Icons/Sig_Settings.ico \
    Icons/Stats.ico \
    Icons/Stop.ico \
    Icons/autosave.ico \
    Icons/autoscale.ico \
    Icons/base.gfie \
    Icons/clear.gfie \
    Icons/esp_icon.gfie \
    Icons/esp_logo_icon.ico \
    Icons/grid.ico \
    Icons/legend.ico \
    Icons/panXY.ico \
    Icons/resetStats.ico \
    Icons/save.ico \
    Icons/title.ico \
    Icons/titleChange.ico \
    Icons/triggerFall.ico \
    Icons/triggerLeft.ico \
    Icons/triggerReset.ico \
    Icons/triggerRight.ico \
    Icons/triggerRise.ico \
    Icons/windows.ico \
    Icons/xTicks.ico \
    Icons/yTicks.ico \
    Icons/zoomBasic.gif \
    Icons/zoomX.ico \
    Icons/zoomXY.ico \
    Icons/zoomY.ico
