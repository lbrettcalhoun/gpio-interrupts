#
# This is the compiler that we'll use
#
CC = xtensa-lx106-elf-gcc

#
# And these are the flags passed to the compiler ... they include the location of include files (.) and the
# -mlongcalls switch.
# Note:  I added an additional custom include for all the non-standard stuff (like uart.h) ... these files are copied
# their respective locations in the SDK into my custom include directory
#
CFLAGS = -I. -I include -mlongcalls -g

# And these are all the options passed to the linker ... mainly which libraries to link (main, net80211, etc).
# All these libraries live in $HOME/esp-open-sdk/sdk/lib and are prefixed with "lib".  Also note that 
# these libraries are nothing more than object files (.o) compressed into an ar archive.
#
LDLIBS = -nostdlib -Wl,--start-group -lmain -lnet80211 -lwpa -llwip -lpp -lphy -lc -Wl,--end-group -lgcc
LDFLAGS = -Teagle.app.v6.ld

# So here is how this works ... when you execute "make" it compiles and assembles (BUT IT DOESN'T LINK) the 
# c files into object (o) files.  Then the linker links the o files together with all the libraries to
# form our executable.  Finally, it uses esptool.py to transform the executable into our 2 binaries which
# make up the firmware.  These actions happen automatically with just "make" based on the chaining. 
user_main-0x00000.bin: user_main
	esptool.py elf2image $^

user_main: user_main.o setup.o

user_main.o: user_main.c

setup.o: setup.c
# This one doesn't get called automatically.  Use "make flash" to actually flash the firmware to the ESP8266
# user_main-0x00000.bin is the boot firmware ... it is uploaded to flash address 0x00000
# user_main-0x10000.bin is our custom firmware ... it is uploaded to flash address 0x10000
# I added a pretty high baud rate so if you get some errors or corruption remove the -b 460800 option
flash: user_main-0x00000.bin
	esptool.py -b 460800 write_flash 0 user_main-0x00000.bin 0x10000 user_main-0x10000.bin

# Use make clean to get rid of the firmware and the executables and the object fles
clean:
	rm -f user_main user_main.o user_main-0x00000.bin user_main-0x10000.bin setup.o
