#include <avr/io.h>
#include <avr/interrupt.h>

/*** MACROS ***/

// tiempos para suministrar agua según nivel de carga 
#define Suministro_de_agua_baja = 1
#define Suministro_de_agua_media = 2
#define Suministro_de_agua_alta = 3

// tiempos para lavar según nivel de carga 
#define lavar_baja = 3
#define lavar_media = 7
#define lavar_alta = 10

// tiempos para enjuagar según nivel de carga 
#define enjuagar_baja = 2 
#define enjuagar_media = 4
#define enjuagar_alta = 5

// tiempos para centrifugar según nivel de carga 
#define centrifugar_baja = 3
#define centrifugar_media = 6
#define centrifugar_alta = 9


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

int segundos = 0; 
int ciclos_tiempo = 0; 

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

    // Se inicializa la variable del tiempo en segundos 
    segundos = 0; 

    // Se habilita interrupción global
    sei();

    // Configuración del temporizador
    TCCR0A = 0x00; // Establecer el Timer0 en modo normal

    TCCR0B = (1 << CS00) | (1 << CS02); // Establecer prescaler a 1024

    TCNT0 = 0; // Inicializar valor del contador del Timer0

    TIMSK |= (1 << TOIE0); // Habilitar la interrupción por desbordamiento del Timer0
}

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

/** Rutina de interrupcion **/

// ISR del Timer0
ISR(TIMER0_OVF_vect) {
    static int contador = 0; // Para contar las interrupciones del temporizador hasta que se alcanza un cierto valor.
    contador++;
    // Cuando contador alcanza 1000, significa que ha pasado aprox 1 segundo, contador se reinicia a 0 y la variable segundos se decrementa en 1.
    if (contador >= 1000) { // Timer0 interrumpe cada 1 ms
        contador = 0;
        segundos--; // tiempo restante para una operación 
        if (segundos <= 0) {
            // Cambiar al siguiente estado de la FSM
            // el tiempo asignado para la fase actual ha finalizado, y la lavadora debe cambiar al siguiente estado
            
        }
    }
}

// Función para configurar el tiempo según el nivel de carga 
void configurarTiempoSuministroDeAgua(TipoCarga carga) {
    switch (carga) {
        case CARGA_BAJA:
            segundos = Suministro_de_agua_baja;
            break;
        case CARGA_MEDIA:
            segundos = Suministro_de_agua_media;
            break;
        case CARGA_ALTA:
            segundos = Suministro_de_agua_alta;
            break;
    }
    estado = SUMINISTRO_DE_AGUA;
}