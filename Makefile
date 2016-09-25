FILENAME    = annoying_labyrinth
#mazeGen/prim_maze_gen.o
OBJECTS     = annoying_labyrinth.o glcd/glcd.o glcd/hal_glcd.o utils/utils.o glcd/font/Standard5x7.o 
OBJECTS		+= renderer.o sdcard/spi.o sdcard/sdcard.o mp3/mp3.o rand/rand.o adc/adc.o mazeGen/prim_maze_gen.o 
OBJECTS		+= wiimote/hal_wt41_fc_uart.o  wiimote/mac.o wiimote/hci.o wiimote/wii_bt.o wiimote/wii_user.o prog_data.o
OBJECTS		+= music_handler.o game_logic.o

MCU         = atmega1280

CCLD        = avr-gcc
CCFLAGS     = -mmcu=$(MCU) -std=gnu99 -Wall -Os -frename-registers
CCFLAGS    += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -fpack-struct -Iwiimote/ -lwiimote -L.
LDFLAGS     = -mmcu=$(MCU) -Wl,-u,vfprintf -lprintf_min

PROG        = avrprog2
PRFLAGS     = -m$(MCU)

all: $(FILENAME).elf

$(FILENAME).elf: $(OBJECTS)
	$(CCLD) $(LDFLAGS) -o $@ $(OBJECTS)

%.o: %.c
	$(CCLD) $(CCFLAGS) -c -o $@ $<

install: $(FILENAME).elf
	$(PROG) $(PRFLAGS) --flash w:$<

verify: $(FILENAME).elf
	$(PROG) $(PRFLAGS) --flash v:$<

clean:
	rm -f $(FILENAME).elf $(OBJECTS)

