
#MCU = attiny85
#MCU = attiny84
MCU = attiny861
PROGRAMMER = usbtiny

CFLAGS  = -mmcu=$(MCU) -Os -Wall
LDFLAGS = -mmcu=$(MCU) -Os

.SUFFIXES: .c .o .py .hex .elf

all: size

.elf.hex: $<
	avr-objcopy -j .text -j .data -O ihex $< $@

organ.elf: organ.c freq.h wave.h
	avr-gcc $(CFLAGS) -o $@ organ.c

size: organ.hex
	avr-size --format=avr --mcu=$(MCU) organ.elf

program: organ.hex
	avrdude -c $(PROGRAMMER) -p $(MCU) -U flash:w:organ.hex

freq.h: freq.py
	python3 freq.py > freq.h

wave.h: wave.py
	python3 wave.py > wave.h

clean:
	rm -f *.elf *.hex freq.h wave.h

