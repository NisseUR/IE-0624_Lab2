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
    // Se habilita interrupción global
    sei();

    // Se configuran entradas y salidas en puerto B
    DDRB = 0xFF;

    // Se inicializan salidas en puerto B
    PORTB = 0x00;

    // Se configuran entradas y salidas en puerto A
    //DDRA 

    // Se inicializan entradas en puerto A
    //DDRB = 0x00;

    // Se configuran entradas y salidas en puerto D
    DDRD = 0x73;

    // Se inicializan entradas en puerto D
    PORTD |= (1 << ISC01 )

    // Se habilita la interrupcion externa por INT0
    GIMSK |= (1 << INT0 ); 

    // EL flanco creciente en INT0 genera la interrupcion
    MCUCR |= (1 << ISC01 )|(1 << ISC00 );

    while(1)
    {


    }
 
}

/*** FUNCIONES ***/
