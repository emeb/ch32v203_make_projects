# Make includes for CH32V203
# 09-20-2023 E. Brombaugh

# Linker script - can be overridden
LDSCRIPT ?= ../misc/CH32V203x8.ld

# Optimization level - can be overridden
COPT ?= -Os

# Compiler Flags
CFLAGS  = $(COPT) -Wall -ffreestanding -flto -nostartfiles -fomit-frame-pointer
CFLAGS += -I. -I../Peripheral/inc -I../Core -I../Startup
CFLAGS += -march=rv32i -mabi=ilp32 
AFLAGS  = -march=rv32i -mabi=ilp32 
LFLAGS  = $(CFLAGS) -nostartfiles -T $(LDSCRIPT) -Wl,-Map=main.map
LFLAGS += -Wl,--gc-sections -Wl,--print-memory-usage
LFLAGS += --specs=nano.specs
CPFLAGS = --output-target=binary
ODFLAGS	= -x --syms

# Executables
TOOLS = /opt/wch/MRS_Toolchain_Linux_x64_V1.70
ARCH = $(TOOLS)/RISC-V\ Embedded\ GCC/bin/riscv-none-embed
CC = $(ARCH)-gcc
LD = $(ARCH)-ld -v
AS = $(ARCH)-as
OBJCPY = $(ARCH)-objcopy
OBJDMP = $(ARCH)-objdump
GDB = $(ARCH)-gdb
NM = $(ARCH)-nm
OPENOCD = $(TOOLS)/OpenOCD/bin/openocd

# Targets
all: main.bin

clean:
	-rm -f $(OBJECTS) crt.lst *.lst *.elf *.bin *.map *.dmp

flash: oocd_flash

oocd_flash: main.elf
	$(OPENOCD) -f ../misc/openocd_ch32.cfg -c "program main.elf verify reset exit"

disassemble: main.elf
	$(OBJDMP) -d main.elf > main.dis

symtab: main.elf
	$(NM) main.elf | sort > main.sym
	
dist:
	tar -c *.h *.c *.s Makefile *.cmd *.cfg openocd_doflash | gzip > minimal_hello_world.tar.gz

main.ihex: main.elf
	$(OBJCPY) --output-target=ihex main.elf main.ihex

main.bin: main.elf 
	$(OBJCPY) $(CPFLAGS) main.elf main.bin
	$(OBJDMP) $(ODFLAGS) main.elf > main.dmp
	ls -l main.elf main.bin

main.elf: $(OBJECTS) $(LDSCRIPT)
	$(CC) $(LFLAGS) -o main.elf $(OBJECTS) -lnosys -lm

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<

