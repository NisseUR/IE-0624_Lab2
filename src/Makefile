FAMILY=-mpic14 # arquitectura del microcontrolador.
MCU=-p12f683 # modelo específico de microcontrolador
CC=sdcc # compilador a utilizar, SDCC (Small Device C Compiler)
LD=sdcc # enlazador a utilizar
CFLAGS=-I. -I/usr/local/share/sdcc/non-free/include
TARGET=lavadora

SRCS = lavadora.c

all:
	${CC} --use-non-free ${FAMILY} ${MCU} ${CFLAGS} -o ${TARGET} ${SRCS}
#	${LD} -o ${TARGET}.elf ${TARGET}.o
#	${OBJCOPY} -j .text -j .data -O ihex ${TARGET}.o ${TARGET}.hex
#	${SIZE} -C --mcu=${MCU} ${TARGET}.elf

clean:
	rm -f *.c~ *.h~ *.o *.elf *.hex *.asm
