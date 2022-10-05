SIFIVE_GCC_PACK ?= no
RISCV_NAME ?= riscv64-unknown-elf
RISCV_PATH ?= /opt/riscv/

MULDIV ?= yes
MABI=ilp32
MARCH := rv32i

ifeq ($(MULDIV),yes)
	MARCH := $(MARCH)m
endif
ifeq ($(COMPRESSED),yes)
	MARCH := $(MARCH)ac
endif

CFLAGS += -march=$(MARCH)  -mabi=$(MABI)
LDFLAGS += -march=$(MARCH)  -mabi=$(MABI)
