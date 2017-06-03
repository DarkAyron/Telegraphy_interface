#                    ___====-_  _-====___
#              _--^^^#####//      \\#####^^^--_
#           _-^##########// (    ) \\##########^-_
#          -############//  |\^^/|  \\############-
#        _/############//   (@::@)   \\############\_
#       /#############((     \\//     ))#############\
#      -###############\\    (oo)    //###############-
#     -#################\\  / VV \  //#################-
#    -###################\\/      \//###################-
#   _#/|##########/\######(   /\   )######/\##########|\#_
#   |/ |#/\#/\#/\/  \#/\##\  |  |  /##/\#/  \/\#/\#/\#| \|
#   `  |/  V  V  `   V  \#\| |  | |/#/  V   '  V  V  \|  '
#      `   `  `      `   / | |  | | \   '      '  '   '
#                       (  | |  | |  )
#                      __\ | |  | | /__
#                     (vvv(VVV)(VVV)vvv)
# 
# Makefile
#
# Compile on a banana pi
#
# Copyright (c) 2016 Ayron
#

#QUOTE = "
MCU = sam3x
ARCH = armv7-m
#TCPREFIX = $(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\arm\arm-gnu-toolchain\bin\arm-none-eabi-
TCPREFIX = arm-none-eabi-
C++ = $(TCPREFIX)g++$(QUOTE)
CC = $(TCPREFIX)gcc$(QUOTE)
AS = $(TCPREFIX)as$(QUOTE)
AR = $(TCPREFIX)ar$(QUOTE)
LD = $(TCPREFIX)ld$(QUOTE)
OBJCOPY = $(TCPREFIX)objcopy$(QUOTE)
OBJDUMP = $(TCPREFIX)objdump$(QUOTE)
DFLAGS = -DDONT_USE_CMSIS_INIT
OFLAGS = 
LFLAGS = -T armelf_sam_eabi.x -mthumb -g
CFLAGS = -mthumb -mlong-calls -std=gnu89 -Ilibsam/cmsis/ARM -Ilibsam/cmsis/Microchip -Ilibsam -Isrc -Isrc/SEGGER -g
WFLAGS = -Wall -Wno-comment -Werror=implicit-int -Werror=implicit-function-declaration -Wno-unused-variable -Wno-unused-function -Wno-unused-but-set-variable
CFLAGS += $(OFLAGS) $(DFLAGS) $(WFLAGS)

OBJC = src/init.o \
	src/main.o \
	src/spi.o \
	src/enc28j60.o \
	src/cycle_counter.o \
	src/ipx.o \
	src/alchemy.o \
	src/anubis.o \
	src/mtask.o \
	src/code.o \
	src/telegraph.o \
	src/dragonusb/usb_core.o \
	src/dragonusb/usb_desc.o \
	src/dragonusb/usb488.o \
	src/SEGGER/SEGGER_RTT.o \
	src/SEGGER/SEGGER_RTT_printf.o \
	src/SEGGER/SEGGER_SYSVIEW.o \
	src/SEGGER/SEGGER_SYSVIEW_Config_NoOS.o \
	libsam/source/efc.o \
	libsam/source/interrupt_sam_nvic.o \
	libsam/source/pio.o \
	libsam/source/pmc.o \
	libsam/source/uotghs.o \
	libsam/source/uotghs_device.o \
	libsam/source/tc.o

OBJS = src/vtables.o

EXEC = telegraphy.elf
BIN = telegraphy.bin
DIS = telegraphy.dis

bin: $(BIN)
exec: $(EXEC)

all: $(BIN) $(DIS)

clean:
	rm -f $(OBJC) $(OBJS) $(EXEC) $(BIN) $(DIS)

dis: $(DIS)

$(DIS): $(EXEC)
	@echo [DISASM] $(EXEC)
	@$(OBJDUMP) -d $(EXEC) > $(DIS)

$(BIN): $(EXEC)
	@echo [OBJCOPY] $@
	@$(OBJCOPY) -O binary $(EXEC) $(BIN)

$(EXEC): $(OBJS) $(OBJC)
	@echo [LD] $@
	@$(CC) -nostartfiles -march=$(ARCH) $(LFLAGS) -o $(EXEC) $(OBJS) $(OBJC)

%.o: %.s
	@echo [AS] $@
	@$(AS) $(SFLAGS) -o $@ $<

%.o: %.c
	@echo [CC] $@
	@$(CC) $(CFLAGS) -march=$(ARCH) -o $@ -c $<
