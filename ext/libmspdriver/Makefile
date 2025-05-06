LIB = libmspdriver

OBJECTS = pmm.o sfr.o framctl_a.o rtc_b.o sysctl.o \
        gpio.o adc12_b.o cs.o lcd_c.o aes256.o \
        tlv.o crc32.o dma.o ram.o wdt_a.o \
        timer_a.o mpy32.o eusci_a_spi.o eusci_b_i2c.o \
        comp_e.o eusci_a_uart.o esi.o timer_b.o crc.o \
        rtc_c.o mpu.o framctl.o ref_a.o eusci_b_spi.o 

override SRC_ROOT = ../../src

override CFLAGS += \
	-I../../src/include \
	-I../../src/include/$(LIB)

include $(MAKER_ROOT)/Makefile.$(TOOLCHAIN)

