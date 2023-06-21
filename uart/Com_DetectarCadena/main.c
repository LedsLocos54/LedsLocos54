//detectar cadenas y leer cadenas

#include <msp430.h> 
#define LONG_BUFFER 200 //tamanno maximo del mensaje

void initUART();
int lectura=0;
int conteo=0; //variable para contar dos desbordes del timer

char mensajon[LONG_BUFFER]; //esta almacena el texto real para procesarlo
char msg_pro[LONG_BUFFER]; //esta procesa
int i_msg=0;

int tamanno=0; //con esta veo en que indice voy del mensaje
int size=0;
volatile int contador=0;
/**
 * main.c
 */

int main(void)
{

    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5; //desabilito alta impedancia

    //pongo el divisor de frecuencia de 1MHZ
    __bis_SR_register(SCG0); //pag 39

    CSCTL3 |= SELREF__REFOCLK;
    CSCTL1 = DCOFTRIMEN | DCOFTRIM0 | DCOFTRIM1 | DCORSEL_0;
    CSCTL2 = FLLD_0+30;
    __delay_cycles(3);
    __bic_SR_register(SCG0);

    CSCTL4 = SELMS__DCOCLKDIV | SELA__REFOCLK;

    /////////////////////////////////////////////////////
    //igual estamos trabajando a 32,000 hz
    P1DIR |= BIT0; //led de salida P1.0

    //configuración de nterrupciones
    TA0CTL |=TASSEL__SMCLK + MC_1 + ID__8; //pag 384, tabla 13-4, se tranaja con smlack, sin divisor de frecuencia y le pusimos el modo ascendente, arranca en 0, interrupcion deshabilitada y TAIFG es la bandera que indica el desborde
    //si te vas a la tabla, el ID es un divisor, el divisor de frecuencia te sieve para contar a más, por ejemplo, ahorita como lo tienes TA=CCR0 se va a desbordar cada segundo, lo máximo que puede es a 2 segundos, pero, si le cambiaras el reloj a un divisor de 2 lo máximo que contarias es hasta 4 seg
    //TA0CCR0 = 0xFFFF;   //le pongo la frecuencia a la mitad del tiempo //registro captura comparacion, el máximo nivel al que cuenta
    //TA0CCTL0 = CCIE; interrupcion captura comparacion
    TA0CTL|=TAIE; //interrumpicion del timer, despues adectro de esta tengo que ver que tipo de interrupcion fue

    initUART(); //inicializa la eUSCI modo UART

    _BIS_SR(GIE);//habilita interrupciones generales
    //__no_operation();//debugger
    while(1);
    return 0;
}

//tabla 1-1 prioridad interrupciones pag 35, pag 281
//pag 293 registro ICCSC, tambien tiene que checar la 298
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER_A0_ISR (void)
{
    //TA0CCTL0 = 0x00;
    UCA0TXBUF = 'z';
}


void initUART(){
    P1SEL1 &= ~BIT4 & ~BIT5; //activa la funcion primaria 10, primero es TX 1.4 y luego es RX 1.5
    P1SEL0 |= BIT4 | BIT5;

    //te vas al capitulo 22, primero se manda el menos significativo, por default son 8 bit por un bit de paro, modo asincrono, seleccionas la fuente SMCLK, por default esta la de la cpu
    //UCA0CTLW0 |= UCPEN;//Habilito bit parifaf
    //UCA0CTLW0 &= ~UCPAR;//mas significaivo primero
    //UCA0CTLW0 |= UCSPB;//DOS BITS DE PARO
    UCA0CTLW0 |= UCSWRST;//activo por software el reset, en realidad es una maqiuna de esrados de la eUSCI modo uart
    UCA0CTLW0 |= UCSSEL__SMCLK;   //FUENTE SMCLK

    //BAUD RATE 9600 BPS CON APROX UN MEGA HERTS, CHECATE LA TABLA 22-5, son bit de paridad 1 bit de paro
    //tabla 22-10 es el UCBR
    UCA0BRW = 6;
    UCA0MCTLW |= UCOS16 | UCBRF_8 | 0x2000;

    //el micro va a leer de la computadora, habilitamos interrrupcion de recepcion
    //lñas cosas que leen se guardan en UCAxRXBUF
    //TX_Din es el registrpo UCAxTXB
    UCA0CTLW0 &= ~UCSWRST;
    UCA0IE |= UCRXIE; //habilito la interrupcion en RX
    //habilitacion reset por software, primero tienes que activarlo luego tiemes que desactivarlo porque si no es bien pinche nena y no va a ajalar


}

void imprimir(const char mensaje[],int longitud){
    TA0CTL|=TACLR;//deshabilito interrupcion
    volatile int i;
    for(i=0;i<longitud;i++){
        while(!(UCA0IFG&UCTXIFG));//si el bufer esta vacio ya puedo transmitir
        UCA0TXBUF = mensaje[i];
        __delay_cycles(10);
    }

        for (i=0;i<2;i=i+1){     //"limpiar pantalla"
            __delay_cycles(10000);
            UCA0TXBUF = 0x0A;
        }

    UCA0TXBUF = '\r'; //retorno de carro
}

#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
    //inicializacion de timer
    TA0CCTL0=CCIE; //habilito interrupcion
    //determinar si llego el dato
    while(!(UCA0IFG&UCTXIFG));//si el bufer esta vacio ya puedo transmitir
    //UCA0TXBUF = UCA0RXBUF; //eco
        if(UCA0RXBUF==36 || size>=LONG_BUFFER){ //se para con $
            //inicializamos el buffer

            for(contador=0;contador<size;contador++){
                msg_pro[contador]=mensajon[contador];
            }
            UCA0TXBUF = 'Y';

            i_msg=size;

            size=0;//reseteas el tamaño
            TA0CCR0 = 0xFFFF;
            imprimir(msg_pro,i_msg);
        }
        else {
             mensajon[size]=UCA0RXBUF;
             size=size+1;
        }


    //UCA0TXBUF = UCA0RXBUF;         //esto es un eco
                                //la de la izquierda es el registro de control de banderas TX por default esta en 1 el bit 1,
}

