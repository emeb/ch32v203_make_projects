# Makefile for CH32C203x8
# 09-16-2023 E. Brombaugh

# sub directories
VPATH = .:../Peripheral/src:../Core:../Startup

# Object files
OBJECTS =   startup_ch32v20x_D8.o core_riscv.o main.o debug.o adc.o \
			system_ch32v20x.o ch32v20x_it.o ch32v20x_misc.o \
			ch32v20x_pwr.o ch32v20x_rcc.o ch32v20x_gpio.o \
			ch32v20x_usart.o ch32v20x_adc.o ch32v20x_dma.o

# Linker script
LDSCRIPT = CH32V203x8.ld

# Compiler Flags
CFLAGS  = -Wall -Os -ffreestanding -flto -nostartfiles -fomit-frame-pointer
CFLAGS += -I. -I../Peripheral/inc -I../Core -I../Startup
CFLAGS += -march=rv32i -mabi=ilp32 
AFLAGS  = -march=rv32i -mabi=ilp32 
LFLAGS  = $(CFLAGS) -nostartfiles -T $(LDSCRIPT) -Wl,-Map=main.map
LFLAGS += -Wl,--gc-sections -Wl,--print-memory-usage
LFLAGS += --specs=nano.specs
CPFLAGS = --output-target=binary
ODFLAGS	= -x --syms
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
	$(OPENOCD) -f openocd_ch32.cfg -c "program main.elf verify reset exit"

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

