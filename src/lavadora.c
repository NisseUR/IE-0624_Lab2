// Laboratorio 2 microcontroladores 
// GPIOs, Timers y FSM: lavadora automatica

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
ISR(INT0)
{
    // Enciendo LED suministro de agua
    PORTB |= (1 << PB7);

}


/*** MAIN ***/

int main(void)
{
    // Se habilita interrupción global
    sei();

    // Se configuran salidas en puerto B
    DDRB = 0xFF;

    // Se configuran entradas

    // Se establece PB7 como salida
    DDRB |= (1 << PB7);

    // Se habilita la interrupcion por INT0
    GIMSK |= (1 << INT0 ); 

    // Interrupción sucede con cualquier cambio 

    while(1)
    {


    }
 
}

/*** FUNCIONES ***/
