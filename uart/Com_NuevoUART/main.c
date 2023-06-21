#include <msp430.h> 
void initUART();
int lectura=0;
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

    //paso 1 llamas la funcion
    initUART(); //inicializa la eUSCI modo UART

        P1DIR |= BIT0; //led de salida P1.0

    //_BIS_SR(GIE);//habilita interrupciones generales
    //__no_operation();//debugger
        int i=0;
        while(1){
            for(i=65;i<122;i++){
                UCA0TXBUF=i;
               __delay_cycles(2000);
            }
        }

    return 0;
}

void initUART(){
    P1SEL1 &= ~BIT4 & ~BIT5; //activa la funcion primaria 10, primero es TX 1.4 y luego es RX 1.5
    P1SEL0 |= BIT4 | BIT5;

    //te vas al capitulo 22, primero se manda el menos significativo, por default son 8 bit por un bit de paro, modo asincrono, seleccionas la fuente SMCLK, por default esta la de la cpu
    UCA0CTLW0 |= UCPEN;//Habilito bit parifaf
    UCA0CTLW0 &= ~UCPAR;//mas significaivo primero
    UCA0CTLW0 |= UCSPB;//DOS BITS DE PARO
    UCA0CTLW0 |= UCSWRST;//activo por software el reset, en realidad es una maqiuna de esrados de la eUSCI modo uart
    UCA0CTLW0 |= UCSSEL__SMCLK;   //FUENTE SMCLK

    //BAUD RATE 9600 BPS CON APROX UN MEGA HERTS, CHECATE LA TABLA 22-5, TIENES UN 0.48 error de que o llegen bien
    //tabla 22-10 es el UCBR
    UCA0BRW = 3;
    UCA0MCTLW |= UCOS16 | UCBRF_4 | 0x0200;

    //el micro va a leer de la computadora, habilitamos interrrupcion de recepcion
    //lñas cosas que leen se guardan en UCAxRXBUF
    //TX_Din es el registrpo UCAxTXB
    UCA0CTLW0 &= ~UCSWRST;
    UCA0IE |= UCRXIE; //habilito la interrupcion en RX
    //habilitacion reset por software, primero tienes que activarlo luego tiemes que desactivarlo porque si no es bien pinche nena y no va a ajalar


}

#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{

    while(!(UCA0IFG&UCTXIFG));//si el bufer esta vacio ya puedo transmitir
    //el while 1 genera retardo
    //UCA0TXBUF = UCA0RXBUF;         //esto es un eco
    //UCA0TXBUF = 'p';
    lectura = UCA0RXBUF;
                                //la de la izquierda es el registro de control de banderas TX por default esta en 1 el bit 1,

    //va a tener dos estados,
    if(lectura==97)
        P1OUT|=BIT0;
    else if(lectura==98)
        P1OUT&=~BIT0;
    else
        P1OUT=P1OUT;
}

