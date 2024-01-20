#include <avr/io.h>
#include <avr/interrupt.h>

/*** MACROS ***/

// tiempos para suministrar agua según nivel de carga 
#define Suministro_de_agua_baja 1
#define Suministro_de_agua_media 2
#define Suministro_de_agua_alta 3

// tiempos para lavar según nivel de carga 
#define lavar_baja 3
#define lavar_media 7
#define lavar_alta 10

// tiempos para enjuagar según nivel de carga 
#define enjuagar_baja 2 
#define enjuagar_media 4
#define enjuagar_alta 5

// tiempos para centrifugar según nivel de carga 
#define centrifugar_baja 3
#define centrifugar_media 6
#define centrifugar_alta 9


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

/*** TIPOS DE CARGA ***/

// Enumeración para los tipos de carga en la lavadora
typedef enum {
    CARGA_CERO,
    CARGA_BAJA,
    CARGA_MEDIA,
    CARGA_ALTA
} TipoCarga;

// Variable para almacenar el tipo de carga actual seleccionado
TipoCarga cargaSeleccionada;

// Se utiliza 8-bit timer

/*** VARIABLES GLOBALES ***/
volatile int boton_on_off = 0;
volatile int boton_carga_alta = 0;
volatile int boton_carga_media = 0;
volatile int boton_carga_baja = 0;
volatile int flag = 0;


int segundos = 0; 
int ciclos_tiempo = 0; 
unsigned int units, decimals;
unsigned int variable_BCD[4];

/*** DECLARACIÓN FUNCIONES ***/

void FSM(void); // Maquina de estados
void configurarTiempoSuministroDeAgua();
void configurarTiempoLavar();
void configurarTiempoEnjuagar();
void configurarTiempoCentrifugar();
void showNumber(int num);
void delay(unsigned int time);
void bcd_convert(int num, unsigned int *bcd);

/*** INTERRUPCIONES ***/

// Interrupción para el botón ON/OFF
ISR(INT0_vect){
    // Boton cambia de estado porque se ha presionado
    boton_on_off=1;
}

// Interrupción por botón carga alta
ISR(PCINT2_vect){
    // Boton cambia de estado porque se ha presionado
    boton_carga_alta = 1;
}

// Interrupción por botón carga media
ISR(INT1_vect){
    // Boton cambia de estado porque se ha presionado
    boton_carga_media = 1;

}

// ISR del Timer0 
ISR(TIMER0_OVF_vect){
    //PORTB |= (1<<PORTB5);
    if(ciclos_tiempo>=128){
        ciclos_tiempo=0;
        segundos--;
        flag=1;

    }else{
        ciclos_tiempo++;
        flag=0;
    }
} 


/*
// ISR del Timer0 
ISR(TIMER0_OVF_vect) {
    static int contador = 0; // Para contar las interrupciones del temporizador hasta que se alcanza un cierto valor.
    contador++;
    // Cuando contador alcanza 1000, significa que ha pasado aprox 1 segundo,
    // contador se reinicia a 0 y la variable segundos se decrementa en 1.
    if(contador >= 1000){ // Timer0 interrumpe cada 1 ms
        contador = 0;
        segundos--; // Tiempo restante para una operación   
        } 
}*/

/*** MAIN ***/

int main(void)
{
    // Se desabilitan las interrupciones
    cli();
    
    // Se configuran entradas y salidas en puerto B
    DDRB = 0xFF;

    // Se inicializan salidas en puerto B
    PORTB = 0x00;

    // Se configuran entradas y salidas en puerto A
    DDRA = 0x00;

    // Se inicializan entradas en puerto A
    PORTA = 0x00;

    // Se configuran entradas y salidas en puerto D
    DDRD = 0x30;

    // Se inicializan entradas en puerto D
    PORTD = 0x00;

    // Se habilita la interrupcion externa para INT0, INT1 y PCINT11
    GIMSK |= (1<<INT0)|(1<<INT1)|(1<<PCIE2); 

    // Se habilita interrupcion por cambio bajo/alto en PCINT11
    PCMSK2 |= (1<<PCINT11);

    // El flanco creciente en INT0 genera la interrupcion
    MCUCR |= (1 << ISC01 )|(1 << ISC00 );

    // El flanco creciente en INT1 genera la interrupcion
    MCUCR |= (1 << ISC11 )|(1 << ISC10 );

    // Se establece el estado inicial de la FSM:
    estado = LAVADORA_APAGADA;

    // Se establece el estado inicial de la carga seleccionada:
    cargaSeleccionada = CARGA_CERO;

    // Se inicializa variable del boton ON/OFF:
    boton_on_off = 0;

    // Se inicializa variable del boton_carga_alta:
    boton_carga_alta = 0;

    // Se inicializa la variable del tiempo en segundos 
    segundos = 0; 

    // Se inicializa la variable del tiempo en segundos 
    ciclos_tiempo = 0; 

    // Configuración del temporizador
    TCCR0A = 0x00; // Establecer el Timer0 en modo normal

    TCCR0B = (1 << CS00) | (1 << CS02); // Establecer prescaler a 1024

    TCNT0 = 0; // Inicializar valor del contador del Timer0

    TIMSK |= (1 << TOIE0); // Habilitar la interrupción por desbordamiento del Timer0

    // Se habilita interrupción global
    sei();

    while(1)
    {
        FSM();
    }
 
}

/*** FUNCIONES ***/
void FSM(){ 
    switch(estado)
    {
        // Estado lavadora apagada
        case(LAVADORA_APAGADA):
            // Se desconecta el display
            PORTD &= ~(1<<PORTD4) & ~(1<<PORTD5);
            //Esperando interrupcion por boton ON/OFF para cambiar de estado
            if( boton_on_off==1 ){
                estado=SELECCIONE_CARGA;
            }
            break;

        // Estado de seleccion de carga
        case (SELECCIONE_CARGA):
            // Se conecta display
            PORTD |= (1<<PORTD4)|(1<<PORTD5);

            // Encender LEDs de modo que se proyecte 00
            PORTB &= ~(1<<PORTB3)&~(1<<PORTB2)&~(1<<PORTB1)&~(1<<PORTB0);

            // Poner en estado bajo el boton ON/OFF
            boton_on_off=0;

            // Esperando interrupcion por boton carga
            if(boton_carga_alta==1){
                cargaSeleccionada = CARGA_ALTA;
                estado = SUMINISTRO_DE_AGUA;
                configurarTiempoSuministroDeAgua(cargaSeleccionada);
            }
            else if (boton_carga_media==1)
            {
                cargaSeleccionada = CARGA_MEDIA;
                estado = SUMINISTRO_DE_AGUA;
                configurarTiempoSuministroDeAgua(cargaSeleccionada);
            }
            break;

        // Estado de suministro de agua
        case (SUMINISTRO_DE_AGUA):

            // Se conecta display
            PORTD |= (1<<PORTD4)|(1<<PORTD5);

            // Se enciende LED modo: Suministro de agua
            PORTB |= (1<<PORTB7);

            // Desplegar el tiempo segun la carga seleccionada
            switch(cargaSeleccionada){
                case (CARGA_ALTA):
                        if(flag){
                            cli();
                            showNumber(segundos);
                            sei();
                            flag = 0;
                        }else if(flag && segundos==0){
                            cli();
                            showNumber(segundos);
                            sei();
                            estado = LAVAR;
                        }
                    break;
                case (CARGA_MEDIA):
                    break;
                case CARGA_BAJA:
                    break;
                case CARGA_CERO:
                    break;
                }
            break;

        // Estado donde se lava la ropa
        case (LAVAR):

            // Se apaga LED modo: Suministro de agua
            PORTB &= ~(1<<PORTB7);

            // Se enciende LED modo: Lavar la ropa
            PORTB |= (1<<PORTB6);

            // Encender LEDs de modo que se proyecte 00
            PORTB &= ~(1<<PORTB3)&~(1<<PORTB2)&~(1<<PORTB1)&~(1<<PORTB0);

            // Aqui falta desplegar el tiempo segun la carga seleccionada
            break;

        // Estado donde se enjuaga la ropa
        case (ENJUAGAR):

            // Se apaga LED modo: Lavar la ropa
            PORTB &= ~(1<<PORTB6);

            // Se enciende LED modo: Enjuagar la ropa
            PORTB |= (1<<PORTB5);

            // Aqui falta desplegar el tiempo segun la carga seleccionada
            break;

        // Estado donde se realizado el centrifugado de la ropa
        case (CENTRIFUGAR):

            // Se apaga LED modo: Enjuagar la ropa
            PORTB &= ~(1<<PORTB5);

            // Se enciende LED modo: Centrifugar la ropa
            PORTB |= (1<<PORTB4);

            // Aqui falta desplegar el tiempo segun la carga seleccionada
            break; 
        default:
            break;

    } 
}

// Función para configurar el tiempo según el nivel de carga 
void configurarTiempoSuministroDeAgua(TipoCarga carga){
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
        case CARGA_CERO:
            break;
    }
    estado = SUMINISTRO_DE_AGUA;
}

// tiempo de lavar 
void configurarTiempoLavar(TipoCarga carga) {
    switch (carga) {
        case CARGA_BAJA:
            segundos = lavar_baja;
            break;
        case CARGA_MEDIA:
            segundos = lavar_media;
            break;
        case CARGA_ALTA:
            segundos = lavar_alta;
            break;
        case CARGA_CERO:
            break;
    }
    estado = LAVAR;
}

// tiempo de enjuagar
void configurarTiempoEnjuagar(TipoCarga carga) {
    switch (carga) {
        case CARGA_BAJA:
            segundos = enjuagar_baja;
            break;
        case CARGA_MEDIA:
            segundos = enjuagar_media;
            break;
        case CARGA_ALTA:
            segundos = enjuagar_alta;
            break;
        case CARGA_CERO:
            break;
    }
    estado = ENJUAGAR;
}

// tiempo de centrifugar
void configurarTiempoCentrifugar(TipoCarga carga) {
    switch (carga) {
        case CARGA_BAJA:
            segundos = centrifugar_baja;
            break;
        case CARGA_MEDIA:
            segundos = centrifugar_media;
            break;
        case CARGA_ALTA:
            segundos = centrifugar_alta;
            break;
        case CARGA_CERO:
            break;
    }
    estado = CENTRIFUGAR;
}

void showNumber(int num) {
    units = num % 10;
    decimals = num / 10;

    // PORTD5 ON
    PORTD|= (1<<PORTD5);

    bcd_convert(decimals, variable_BCD);
    PORTB |= (variable_BCD[0]<<PORTB3)|(variable_BCD[1]<<PORTB2)|(variable_BCD[2]<<PORTB1)|(variable_BCD[3]<<PORTB0);
    delay(1);

    // PORTD5 OFF
    PORTD &= ~(1<<PORTD5);
    // PORTD4 ON
    PORTD|= (1<<PORTD4); 

    bcd_convert(units, variable_BCD);
    PORTB |= (variable_BCD[0]<<PORTB3)|(variable_BCD[1]<<PORTB2)|(variable_BCD[2]<<PORTB1)|(variable_BCD[3]<<PORTB0);

    //delay(1);
    // PORTD4 OFF
    PORTD &= ~(1<<PORTD4);

}

void delay(unsigned int time) {
    unsigned int i, j;
    for (i = 0; i < time; i++) {
        for (j = 0; j < 1275; j++);
    }
}

void bcd_convert(int num, unsigned int *bcd) {
    bcd[0] = num / 8;
    bcd[1] = (num % 8) / 4;
    bcd[2] = (num % 4) / 2;
    bcd[3] = num % 2;
}
