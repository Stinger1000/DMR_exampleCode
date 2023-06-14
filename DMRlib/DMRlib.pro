QT -= gui

QT += core network

TEMPLATE = lib
DEFINES += IDMRLIB_LIBRARY

CONFIG += c++17 usb

linux {
    # for Intel VTune Profiler
    QMAKE_CXXFLAGS += -g3 -gdwarf-2
    QMAKE_CXXFLAGS_RELEASE += -g
    QMAKE_CFLAGS_RELEASE += -g

#    CONFIG += sanitizer sanitize_address
#    LIBS += -lasan
}

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    DMRlib.cpp \
    IDMRlib.cpp \
    controllers/AccessController.cpp \
    controllers/ClockDetuneController.cpp \
    #controllers/DMRReceiverController.cpp \
    controllers/DumpDebugController.cpp \
    controllers/FlashFirmwareController.cpp \
    controllers/MainController.cpp \
    controllers/SpectrumController.cpp \
    controllers/SpectrumScanController.cpp \
    controllers/StatusController.cpp \
    #dmr/dmr_sample_processor.cpp \
    utils/DumpDevice.cpp \
    utils/DumpWriter.cpp \
    utils/EthernetDevice.cpp \
    utils/RawPacker.cpp \
    utils/ThresholdUtils.cpp \
    utils/USBDevice.cpp \
    utils/USBPacketCollector.cpp \
    utils/blercounter.cpp \
    utils/ethernetsocket.cpp \
    utils/usbsocket.cpp

HEADERS += \
    DMRlib.h \
    IDMRlib.h \
    IDMRlib_global.h \
    controllers/AccessController.h \
    controllers/ClockDetuneController.h \
    #controllers/DMRReceiverController.h \
    controllers/DumpDebugController.h \
    controllers/FlashFirmwareController.h \
    controllers/MainController.h \
    controllers/SpectrumController.h \
    controllers/SpectrumScanController.h \
    controllers/StatusController.h \
    #dmr/dmr_sample_processor.h \
    models/HostModel.h \
    protocol.h \
    utils/DumpDevice.h \
    utils/DumpWriter.h \
    utils/ErrorCounter.h \
    utils/EthernetDevice.h \
    utils/IDevice.h \
    utils/RawPacker.h \
    utils/ThresholdUtils.h \
    utils/USBDevice.h \
    utils/USBPacketCollector.h \
    utils/bit_stream.h \
    utils/blercounter.h \
    utils/ethernetsocket.h \
    utils/usbsocket.h

# Default rules for deployment.

#INCLUDEPATH += 3rdparty/sdrangel_sdrbase
#INCLUDEPATH += 3rdparty/dsdcc

win32: INCLUDEPATH += 3rdparty/libusb-win32/include/libusb-1.0

win32: LIBS += -L$${PWD}/3rdparty/libusb-win32/MinGW64/dll -llibusb-1.0
unix: LIBS += -lusb-1.0

unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target
