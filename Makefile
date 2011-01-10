# Makefile

PROG=			tsxx

SRCS=			\
			src/tsxx/exceptions/stdio_error.cpp \
			main.cpp

INCDIRS=		include
CROSS_COMPILE=		arm-linux-gnu-

include mk/build.mk
