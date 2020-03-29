PROJ_NAME=briey
DEBUG=yes
BENCH=no
MULDIV=yes

SRCS = 	$(wildcard src/*.c) \
	$(wildcard src/*.cpp)  \
	$(wildcard src/*.S)

LDSCRIPT = libs/briey.ld

INC += -Ilibs/

include resources/gcc.mk
include resources/subproject.mk		
