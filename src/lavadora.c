#include <avr/io.h>
#include <avr/interrupt.h>

/*** MACROS ***/


/*** ESTADOS ***/

// Se enumeran los estados de la lavadora
typedef enum {
    LAVADORA_APAGADA,
    SELECCIONE_CARGA,
    SUMINISTRO_DE_AGUA,
    LAVAR,
    ENJUAGAR,
    CENTRIFUGAR
} ESTADO;

// Variable de estado actual
ESTADO estado;

// Se utiliza 8-bit timer

/*** VARIABLES GLOBALES ***/


/*** DECLARACIÓN FUNCIONES ***/


/*** INTERRUPCIONES ***/

// Interrupción para el botón ON/OFF
ISR(INT0_vect){
    // Enciendo LED suministro de agua
    PORTB |= (1 << PB7);
}

/*** MAIN ***/

int main(void)
{
    // Se configuran entradas y salidas en puerto B
    DDRB = 0xFF;

    // Se inicializan salidas en puerto B
    PORTB = 0x00;

    // Se configuran entradas y salidas en puerto A
    DDRA = 0xFF;

    // Se inicializan entradas en puerto A
    PORTA = 0x00;

    // Se configuran entradas y salidas en puerto D
    DDRD = 0x73;

    // Se inicializan entradas en puerto D
    PORTD |= (0<<PORTD6)|(0<<PORTD5)|(0<<PORTD4)|(0<<PORTD3)|(0<<PORTD2)|(0<<PORTD2)|(0<<PORTD1)|(0<<PORTD0);
    
    // Se habilita la interrupcion externa para INT0
    GIMSK |= (1<<INT0); 

    // EL flanco creciente en INT0 genera la interrupcion
    MCUCR |= (1 << ISC01 )|(1 << ISC00 );

    // Se habilita interrupción global
    sei();

    while(1)
    {


    }
 
}

/*** FUNCIONES ***/
