# Makefile for CH32C203x8
# 09-16-2023 E. Brombaugh

# sub directories
VPATH = .:../Peripheral/src:../Core:../Startup

# Object files
OBJECTS =   startup_ch32v20x_D8.o core_riscv.o main.o debug.o i2s.o \
			system_ch32v20x.o ch32v20x_it.o ch32v20x_misc.o \
			ch32v20x_pwr.o ch32v20x_rcc.o ch32v20x_gpio.o ch32v20x_usart.o \
			ch32v20x_dma.o ch32v20x_spi.o
			
# Optional Linker script override
#LDSCRIPT = ../misc/CH32V203x8.ld

# Optional Optimization level override
#COPT = -O3

include ../misc/makefile.mk

