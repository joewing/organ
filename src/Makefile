
#MCU = attiny85
#MCU = attiny45
#MCU = attiny25
#MCU = attiny84
MCU = attiny861a
PROGRAMMER = usbtiny

CFLAGS  = -mmcu=$(MCU) -Os -Wall
LDFLAGS = -mmcu=$(MCU) -Os

.SUFFIXES: .c .o .py .hex .elf

all: size

.elf.hex: $<
	avr-objcopy -j .text -j .data -O ihex $< $@

mk.elf: mk.c freq.h wave.h
	avr-gcc $(CFLAGS) -o $@ mk.c

size: mk.hex
	avr-size --format=avr --mcu=$(MCU) mk.elf

program: mk.hex
	avrdude -c $(PROGRAMMER) -p $(MCU) -U flash:w:mk.hex

freq.h: freq.py
	python3 freq.py > freq.h

wave.h: wave.py
	python3 wave.py > wave.h

clean:
	rm -f *.elf *.hex freq.h wave.h

