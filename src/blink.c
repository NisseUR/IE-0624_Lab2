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
volatile int boton_on_off = 0;


/*** DECLARACIÓN FUNCIONES ***/

// Maquina de estados
void FSM(void);

/*** INTERRUPCIONES ***/

// Interrupción para el botón ON/OFF
ISR(INT0_vect){
    // Boton cambia de estado porque se ha presionado
    boton_on_off=1;
}

/*** MAIN ***/

int main(void)
{

    cli();
    
    // Se configuran entradas y salidas en puerto B
    DDRB = 0xFF;

    // Se inicializan salidas en puerto B
    PORTB = 0x00;

    // Se configuran entradas y salidas en puerto A
    DDRA = 0x04;

    // Se inicializan entradas en puerto A
    PORTA = 0x00;

    // Se configuran entradas y salidas en puerto D
    DDRD = 0x7A;

    // Se inicializan entradas en puerto D
    //PORTD &= (0<<PORTD6)|(0<<PORTD5)|(0<<PORTD4)|(0<<PORTD3)|(0<<PORTD2)|(0<<PORTD2)|(0<<PORTD1)|(0<<PORTD0);
    PORTD = 0x00;

    // Se habilita la interrupcion externa para INT0
    GIMSK |= (1<<INT0); 

    // EL flanco creciente en INT0 genera la interrupcion
    MCUCR |= (1 << ISC01 )|(1 << ISC00 );

    // Se establece el estado inicial de la FSM:
    estado = LAVADORA_APAGADA;

    // Se inicializa variable del boton ON/OFF:
    boton_on_off = 0;

    // Se habilita interrupción global
    sei();

    while(1)
    {
        FSM();
    }
 
}

/*** FUNCIONES ***/
void FSM()
{ 
    switch(estado)
    {
        // Estado lavadora apagada
        case(LAVADORA_APAGADA):
        // Se desconecta el display
        PORTD &= ~(1<<PORTD4) & ~(1<<PORTD5);
        //PORTD |= (1<<PORTD4) | (1<<PORTD5);
        //Esperando interrupcion por boton ON/OFF para cambiar de estado
        
        if( boton_on_off==1 ){
            estado=SELECCIONE_CARGA;
            PORTB |= (1<<PORTB7);
        }
        break;

        // Estado de seleccion de carga
         case (SELECCIONE_CARGA):
            // Se conecta display
            //PORTD |= (1<<PORTD4)|(1<<PORTD5);
            PORTD |= (1<<PORTD4);

            // Encender LEDs de modo que se proyecte 00
            PORTB &= ~(1<<PORTB3)&~(1<<PORTB2)&~(1<<PORTB1)&~(1<<PORTB0);

            // Poner en estado bajo el boton ON/OFF
            boton_on_off=0;
            // Aqui falta desplegar el tiempo segun la carga seleccionada
            break;

        // Estado de suministro de agua
        case (SUMINISTRO_DE_AGUA):
            break;

        // Estado donde se lava la ropa
        case (LAVAR):
            break;

        // Estado donde se enjuaga la ropa
        case (ENJUAGAR):
            break;

        // Estado donde se realizado el centrifugado de la ropa
        case (CENTRIFUGAR):
            break; 
        
        default:
            estado = LAVADORA_APAGADA;
            break;

    } 
}
