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
volatile int pausa = 1;
volatile int boton_carga_alta = 0;
volatile int boton_carga_media = 0;
volatile int boton_carga_baja = 0;
volatile int aux1 = 0, aux2 = 0, flag_pausa=0;


int segundos = 0; 
int ciclos_tiempo = 0; 
unsigned int units, decimals;
unsigned int variable_BCD[4];
int flag = 0; 

/*** DECLARACIÓN FUNCIONES ***/

void FSM(void); // Maquina de estados
void configurarTiempoSuministroDeAgua();
void configurarTiempoLavar();
void configurarTiempoEnjuagar();
void configurarTiempoCentrifugar();
void showNumber(int num);

/*** INTERRUPCIONES ***/

// Interrupción para el botón ON/OFF
ISR(INT0_vect){
    //Pausa
    if(boton_on_off==1){
        if(pausa ==0){
            pausa = 1;
            //PORTD |= (1<<PORTD1);
        }else{
            pausa = 0;
            flag_pausa = 1;
        }
    }
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

// Interrupción por botón carga baja
ISR(PCINT1_vect){
    // Boton cambia de estado porque se ha presionado
    boton_carga_baja = 1;
}

ISR(TIMER0_COMPA_vect){

    if(boton_carga_alta || boton_carga_baja || boton_carga_media){
        ciclos_tiempo++;
        // MCU opera en simulador a 16MHz y se prescala a razon de 
        // 1024. 16.384 ms para que TCNT0 = OCR0A = 0xFF. 
        // Por lo que, 62 iteraciones completan 1s aprox.
        if(ciclos_tiempo >= 62){ // Timer0 interrumpe cada 1 ms
            ciclos_tiempo = 0;
            segundos--; // Tiempo restante para una operación   
        } 

    }
    
}


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
    //DDRD = 0x30;
    DDRD = 0x72;

    // Se inicializan entradas en puerto D
    PORTD = 0x00;

    // Se habilita la interrupcion externa para INT0, INT1, PCINT11 y PCINT8
    GIMSK |= (1<<INT0)|(1<<INT1)|(1<<PCIE2)|(1<<PCIE1); 

    // Se habilita interrupcion por cambio bajo/alto en PCINT11
    PCMSK2 |= (1<<PCINT11);

    // Se habilita interrupcion por cambio bajo/alto en PCINT8
    PCMSK1 |= (1<<PCINT8);

    // El flanco creciente en INT0 genera la interrupcion
    MCUCR |= (1 << ISC01 )|(1 << ISC00 );

    // El flanco creciente en INT1 genera la interrupcion
    MCUCR |= (1 << ISC11 )|(1 << ISC10 );

    // Se establece el estado inicial de la FSM:
    estado = LAVADORA_APAGADA;

    // Se establece el estado inicial de la carga seleccionada:
    cargaSeleccionada = CARGA_CERO;

    // Se inicializan botones:
    boton_on_off = 0;
    boton_carga_alta = 0; 
    boton_carga_media = 0;
    boton_carga_baja = 0;

    // Se inicializa la variable del tiempo en segundos 
    segundos = 0; 

    // Se inicializa la variable del tiempo en segundos 
    ciclos_tiempo = 0; 

    // Configuración del temporizador
    TCCR0A = 0x00; // Establecer el Timer0 en modo normal

    TCCR0B = (1 << CS00) | (1 << CS02); // Establecer prescaler a 1024

    TCNT0 = 0; // Inicializar valor del contador del Timer0

    TIMSK |= 0b1;

    OCR0A = 0xFF;

    flag = 0;

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

            // Se apaga LED modo: Centrigufar
            PORTB &= ~(1<<PORTB4);

            //Esperando interrupcion por boton ON/OFF para cambiar de estado
            if( boton_on_off==1 ){
                estado=SELECCIONE_CARGA;
            }
            break;

        // Estado de seleccion de carga
        case (SELECCIONE_CARGA):
            // Se conecta display
            PORTD |= (1<<PORTD5);

            // Encender LEDs de modo que se proyecte 00
            PORTB &= ~(1<<PORTB3)&~(1<<PORTB2)&~(1<<PORTB1)&~(1<<PORTB0);

            // Poner en estado bajo el boton ON/OFF
           //boton_on_off=0;

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
             else if (boton_carga_baja==1)
            {
                cargaSeleccionada = CARGA_BAJA;
                estado = SUMINISTRO_DE_AGUA;
                configurarTiempoSuministroDeAgua(cargaSeleccionada);
            }
            break;

        // Estado de suministro de agua
        case (SUMINISTRO_DE_AGUA):

            // Se elimina una cifra del display
            PORTD &= ~(1<<PORTD4);

            // Se enciende LED modo: Suministro de agua
            PORTB |= (1<<PORTB7);

            // Desplegar el tiempo segun la carga seleccionada
            switch(cargaSeleccionada){
                case (CARGA_ALTA):
                    while(pausa){
                        cli();
                        showNumber(segundos);
                        sei();
                        if(segundos<0){
                            configurarTiempoLavar(cargaSeleccionada);
                            estado = LAVAR;
                            break;
                        }
                    }
                    if(pausa==0 && flag_pausa == 1){
                        aux1 = segundos;
                        aux2 = ciclos_tiempo;
                        flag_pausa = 0;
                    }else if(pausa==0){
                        segundos = aux1;
                        ciclos_tiempo = aux2;
                    }
                    break;
                case (CARGA_MEDIA):
                    while(pausa){
                            cli();
                            showNumber(segundos);
                            sei();
                            if(segundos<0){
                                configurarTiempoLavar(cargaSeleccionada);
                                estado = LAVAR;
                                break;
                            }
                        }
                        if(pausa==0 && flag_pausa == 1){
                            aux1 = segundos;
                            aux2 = ciclos_tiempo;
                            flag_pausa = 0;
                        }else if(pausa==0){
                            segundos = aux1;
                            ciclos_tiempo = aux2;
                        }
                        break;
                case (CARGA_BAJA):
                        while(pausa){
                            cli();
                            showNumber(segundos);
                            sei();
                            if(segundos<0){
                                configurarTiempoLavar(cargaSeleccionada);
                                estado = LAVAR;
                                break;
                            }
                        }
                        if(pausa==0 && flag_pausa == 1){
                            aux1 = segundos;
                            aux2 = ciclos_tiempo;
                            flag_pausa = 0;
                        }else if(pausa==0){
                            segundos = aux1;
                            ciclos_tiempo = aux2;
                        }
                        break;
                case (CARGA_CERO):
                    break;
                    

                }
            break;

                                                                            /***    ESTADO LAVAR ROPA    ***/ 
        case (LAVAR):

            // Se apaga LED modo: Suministro de agua
            PORTB &= ~(1<<PORTB7);

            // Se enciende LED modo: Lavar la ropa
            PORTB |= (1<<PORTB6);

            // Desplegar el tiempo segun la carga seleccionada
            switch(cargaSeleccionada){
                case (CARGA_ALTA):
                    while(pausa){
                                cli();
                                showNumber(segundos);
                                sei();
                                if(segundos<0){
                                    configurarTiempoEnjuagar(cargaSeleccionada);
                                    estado = ENJUAGAR;
                                    break;
                                }
                            }
                            if(pausa==0 && flag_pausa == 1){
                                aux1 = segundos;
                                aux2 = ciclos_tiempo;
                                flag_pausa = 0;
                            }else if(pausa==0){
                                segundos = aux1;
                                ciclos_tiempo = aux2;
                            }
                    break;
                case (CARGA_MEDIA):
                        while(pausa){
                                cli();
                                showNumber(segundos);
                                sei();
                                if(segundos<0){
                                    configurarTiempoEnjuagar(cargaSeleccionada);
                                    estado = ENJUAGAR;
                                    break;
                                }
                            }
                            if(pausa==0 && flag_pausa == 1){
                                aux1 = segundos;
                                aux2 = ciclos_tiempo;
                                flag_pausa = 0;
                            }else if(pausa==0){
                                segundos = aux1;
                                ciclos_tiempo = aux2;
                            }
                    break;
                case (CARGA_BAJA):
                        while(pausa){
                                cli();
                                showNumber(segundos);
                                sei();
                                if(segundos<0){
                                    configurarTiempoEnjuagar(cargaSeleccionada);
                                    estado = ENJUAGAR;
                                    break;
                                }
                            }
                            if(pausa==0 && flag_pausa == 1){
                                aux1 = segundos;
                                aux2 = ciclos_tiempo;
                                flag_pausa = 0;
                            }else if(pausa==0){
                                segundos = aux1;
                                ciclos_tiempo = aux2;
                            }
                    break;
                case (CARGA_CERO):
                    break;
                }
            break;

                                                                            /***    ESTADO ENJUAGAR ROPA    ***/ 
        case (ENJUAGAR):

            // Se apaga LED modo: Lavar la ropa
            PORTB &= ~(1<<PORTB6);

            // Se enciende LED modo: Enjuagar la ropa
            PORTB |= (1<<PORTB5);

             // Desplegar el tiempo segun la carga seleccionada
            switch(cargaSeleccionada){
                case (CARGA_ALTA):
                    while(pausa){
                        cli();
                        showNumber(segundos);
                        sei();
                        if(segundos<0){
                                configurarTiempoCentrifugar(cargaSeleccionada);
                                estado = CENTRIFUGAR;
                                break;
                        }
                    }
                    if(pausa==0 && flag_pausa == 1){
                        aux1 = segundos;
                        aux2 = ciclos_tiempo;
                        flag_pausa = 0;
                    }else if(pausa==0){
                        segundos = aux1;
                        ciclos_tiempo = aux2;
                    }
                break;
                                    
                case (CARGA_MEDIA):
                        while(pausa){
                        cli();
                        showNumber(segundos);
                        sei();
                        if(segundos<0){
                                configurarTiempoCentrifugar(cargaSeleccionada);
                                estado = CENTRIFUGAR;
                                break;
                        }
                    }
                    if(pausa==0 && flag_pausa == 1){
                        aux1 = segundos;
                        aux2 = ciclos_tiempo;
                        flag_pausa = 0;
                    }else if(pausa==0){
                        segundos = aux1;
                        ciclos_tiempo = aux2;
                    }
                break;
                case (CARGA_BAJA):
                        while(pausa){
                        cli();
                        showNumber(segundos);
                        sei();
                        if(segundos<0){
                                configurarTiempoCentrifugar(cargaSeleccionada);
                                estado = CENTRIFUGAR;
                                break;
                        }
                    }
                    if(pausa==0 && flag_pausa == 1){
                        aux1 = segundos;
                        aux2 = ciclos_tiempo;
                        flag_pausa = 0;
                    }else if(pausa==0){
                        segundos = aux1;
                        ciclos_tiempo = aux2;
                    }
                break;
                case (CARGA_CERO):
                    break;
                }
            break;

                                                                            /***    ESTADO CENTRIFUGAR ROPA    ***/ 
        case (CENTRIFUGAR):

            // Se apaga LED modo: Enjuagar la ropa
            PORTB &= ~(1<<PORTB5);

            // Se enciende LED modo: Centrifugar la ropa
            PORTB |= (1<<PORTB4);

            // Desplegar el tiempo segun la carga seleccionada
            switch(cargaSeleccionada){
                case (CARGA_ALTA):
                    while(pausa){
                            cli();
                            showNumber(segundos);
                            sei();
                            if(segundos<0){
                                    estado = LAVADORA_APAGADA;
                                    boton_on_off=0;
                                    break;
                            }
                        }
                        if(pausa==0 && flag_pausa == 1){
                            aux1 = segundos;
                            aux2 = ciclos_tiempo;
                            flag_pausa = 0;
                        }else if(pausa==0){
                            segundos = aux1;
                            ciclos_tiempo = aux2;
                        }
                    break;
                case (CARGA_MEDIA):
                        while(pausa){
                            cli();
                            showNumber(segundos);
                            sei();
                            if(segundos<0){
                                    estado = LAVADORA_APAGADA;
                                    boton_on_off=0;
                                    break;
                            }
                        }
                        if(pausa==0 && flag_pausa == 1){
                            aux1 = segundos;
                            aux2 = ciclos_tiempo;
                            flag_pausa = 0;
                        }else if(pausa==0){
                            segundos = aux1;
                            ciclos_tiempo = aux2;
                        }
                    break;
                case (CARGA_BAJA):
                        while(pausa){
                            cli();
                            showNumber(segundos);
                            sei();
                            if(segundos<0){
                                    estado = LAVADORA_APAGADA;
                                    boton_on_off=0;
                                    break;
                            }
                        }
                        if(pausa==0 && flag_pausa == 1){
                            aux1 = segundos;
                            aux2 = ciclos_tiempo;
                            flag_pausa = 0;
                        }else if(pausa==0){
                            segundos = aux1;
                            ciclos_tiempo = aux2;
                        }
                    break;
                case (CARGA_CERO):
                    break;
                }
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
}

void showNumber(int num) {
    if(num == 10){ 
        PORTB = (PORTB & 0xF0) | 0x00;
        PORTD |= (1<<PORTD4);
    }else if(num == 9){
        PORTD &= ~(1<<PORTD4); // Apago decimal display
        PORTB = (PORTB & 0xF0) | 0x09;
    }else if(num == 8){
        PORTB = (PORTB & 0xF0) | 0x08;
    }else if(num == 7){
        PORTB = (PORTB & 0xF0) | 0x07;
    }else if(num == 6){
        PORTB = (PORTB & 0xF0) | 0x06;
    }else if(num == 5){
        PORTB = (PORTB & 0xF0) | 0x05;
    }else if(num == 4){
        PORTB = (PORTB & 0xF0) | 0x04;
    }else if(num == 3){
        PORTB = (PORTB & 0xF0) | 0x03;
    }else if(num == 2){
        PORTB = (PORTB & 0xF0) | 0x02;
    }else if(num == 1){
        PORTB = (PORTB & 0xF0) | 0x01;
    }else if(num == 0){
        PORTB = (PORTB & 0xF0) | 0x00;
    }


}
