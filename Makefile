# Makefile

LIB=			tsxx

SRCS=			\
			src/tsxx/exceptions/stdio_error.cpp \
			src/tsxx/registers/reg8.cpp \
			src/tsxx/registers/reg16.cpp \
			src/tsxx/registers/reg32.cpp \
			src/tsxx/system/file_descriptor.cpp \
			src/tsxx/system/memory.cpp \
			src/tsxx/system/memory_region.cpp \
			src/tsxx/system/memory_region_window.cpp \
			src/tsxx/ts7300/board.cpp \
			src/tsxx/ts7300/devices/lcd.cpp \
			src/tsxx/ts7300/devices/spi.cpp \
			src/tsxx/ts7300/devices/xdio.cpp

INCDIRS=		include
CROSS_COMPILE=		arm-linux-gnu-

include mk/build.mk
