
MCU = attiny85
PROGRAMMER = usbtiny

CFLAGS  = -mmcu=$(MCU) -Os -Wall
LDFLAGS = -mmcu=$(MCU) -Os

.SUFFIXES: .c .o .py .hex .elf

all: dp-size

.elf.hex: $<
	avr-objcopy -j .text -j .data -O ihex $< $@

dp.elf: dp.c freq.h wave.h
	avr-gcc $(CFLAGS) -o $@ dp.c

dp-size: dp.hex
	avr-size --format=avr --mcu=$(MCU) dp.elf

dp-program: dp.hex
	avrdude -c $(PROGRAMMER) -p $(MCU) -U flash:w:dp.hex

freq.h: freq.py
	python3 freq.py > freq.h

wave.h: wave.py
	python3 wave.py > wave.h

clean:
	rm -f *.elf *.hex freq.h wave.h

