#-------------------------------------------------
#
# Project created by QtCreator 2015-02-11T14:07:58
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets serialport

TARGET = blackchirp
TEMPLATE = app

CONFIG += c++11

#Enable CUDA GPU support
CONFIG += gpu-cuda

#Run with virtual hardware
#CONFIG += nohardware

SOURCES += main.cpp

RESOURCES += resources.qrc

unix:!macx: LIBS += -lqwt -lgsl -lm -lgslcblas

gpu-cuda {
	DEFINES += BC_CUDA

	# Cuda sources
	CUDA_SOURCES += gpuaverager.cu

	# Path to cuda toolkit install
	CUDA_DIR      = /usr/local/cuda-7.0
	INCLUDEPATH  += $$CUDA_DIR/include -isystem /usr/include/qt5 -isystem /usr/include/qt5/QtCore
	QMAKE_LIBDIR += $$CUDA_DIR/lib64
	CUDA_INC = $$join(INCLUDEPATH,' -I','-I',' ')
	CUDA_INT
	LIBS += -L$$CUDA_DIR/lib64 -lcuda -lcudart
	# GPU architecture
	CUDA_ARCH     = sm_50
	NVCCFLAGS     = --compiler-options -use_fast_math --ptxas-options=-v -Xcompiler -fPIE -Xcompiler -Wno-attributes

	CONFIG(debug, debug|release) {
		cuda_d.commands = $$CUDA_DIR/bin/nvcc -D_DEBUG -std=c++11 -m64 -O3 -arch=$$CUDA_ARCH -c $$NVCCFLAGS \
					   $$CUDA_INC ${QMAKE_FILE_NAME} -o ${QMAKE_FILE_OUT} \
					   2>&1 | sed -r \"s/\\(([0-9]+)\\)/:\\1/g\" 1>&2

		cuda_d.dependency_type = TYPE_C
		cuda_d.depend_command = $$CUDA_DIR/bin/nvcc -O3 -M $$CUDA_INC $$NVCCFLAGS   ${QMAKE_FILE_NAME}

		cuda_d.input = CUDA_SOURCES
		cuda_d.output = ${OBJECTS_DIR}${QMAKE_FILE_BASE}_cuda.o
		QMAKE_EXTRA_COMPILERS += cuda_d
	}
	else {
		cuda.commands = $$CUDA_DIR/bin/nvcc -std=c++11 -m64 -O3 -arch=$$CUDA_ARCH -c $$NVCCFLAGS \
					 $$CUDA_INC $$LIBS ${QMAKE_FILE_NAME} -o ${QMAKE_FILE_OUT} \
					 2>&1 | sed -r \"s/\\(([0-9]+)\\)/:\\1/g\" 1>&2

		cuda.dependency_type = TYPE_C
		cuda.depend_command = $$CUDA_DIR/bin/nvcc -O3 -M $$CUDA_INC $$NVCCFLAGS   ${QMAKE_FILE_NAME}

		cuda.input = CUDA_SOURCES
		cuda.output = ${OBJECTS_DIR}${QMAKE_FILE_BASE}_cuda.o
		QMAKE_EXTRA_COMPILERS += cuda
	}
}


nohardware {
#The values here should not be modified
FTMWSCOPE=0
AWG=0
SYNTH=0
PGEN=0
PGEN_GAS=0
PGEN_AWG=1
PGEN_XMER=2
PGEN_LIF=3
PGEN_CHANNELS=8
FC=0
FC_CHANNELS=4
LIFSCOPE=0
IOBOARD=0
} else {
#------------------------------------------------
# The following defines select hardware implementations.
# -----------------------------------------------

# FTMW Oscilloscope (0 = virtual, 1 = DSA71604C)
FTMWSCOPE=0

#AWG (0 = virtual, 1 = AWG 70002A)
AWG=0

#Synth (0 = virtual, 1 = Valon 5009)
SYNTH=1

#Pulse generator (0 = virtual, 1 = Quantum Composers 9528+)
PGEN=0
#pulse generator channel definitions (0 = A, 1 = B, etc...)
PGEN_GAS=0
PGEN_AWG=1
PGEN_XMER=2
PGEN_LIF=3
#num channels
PGEN_CHANNELS=8

#Flow Controller (0 = virtual, 1 = MKS 647C)
FC=0
#num channels
FC_CHANNELS=4

#LIF Oscilloscope (0 = virtual, 1 = DPO3012)
LIFSCOPE=0

#IO Board (0 = virtual, 1 = Labjack U3)
IOBOARD=0
}


include(acquisition.pri)
include(gui.pri)
include(data.pri)
include(hardware.pri)
include(wizard.pri)
include(implementations.pri)


#------------------------------------------------
# Do not modify the following
# -----------------------------------------------

DEFINES += BC_FTMWSCOPE=$$FTMWSCOPE
DEFINES += BC_AWG=$$AWG
DEFINES += BC_SYNTH=$$SYNTH
DEFINES += BC_PGEN=$$PGEN BC_PGEN_GASCHANNEL=$$PGEN_GAS BC_PGEN_AWGCHANNEL=$$PGEN_AWG BC_PGEN_XMERCHANNEL=$$PGEN_XMER BC_PGEN_LIFCHANNEL=$$PGEN_LIF BC_PGEN_NUMCHANNELS=$$PGEN_CHANNELS
DEFINES += BC_FLOWCONTROLLER=$$FC  BC_FLOW_NUMCHANNELS=$$FC_CHANNELS
DEFINES += BC_LIFSCOPE=$$LIFSCOPE
DEFINES += BC_IOBOARD=$$IOBOARD

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3

equals(FTMWSCOPE,0) {
RESOURCES += virtualdata.qrc
}

DISTFILES += \
    52-serial.rules


