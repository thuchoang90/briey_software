PROJ_NAME=briey
DEBUG=no
BENCH=no
MULDIV=yes

CFLAGS += -fno-inline -fno-common 
LDFLAGS += -lc

CFLAGS +=  -DPREALLOCATE=1 -DHOST_DEBUG=0 -DMSC_CLOCK 

SRCS = 	src/briey_main.c src/briey_crt.S  src/memcpy.c src/aes.c #src/stdlib.c # src/dhry_1.c 

LDFLAGS += -lc #Used to get memcpy/strcmp stuff only

LDSCRIPT = libs/briey.ld

INC += -Ilibs/

include resources/gcc.mk
include resources/subproject.mk		
