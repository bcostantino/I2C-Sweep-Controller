/////////////////////////////////////////////////////////////////
//***************************************************************
//
//  uartio/uartio.c
//  register level c code for MSP430G2553 UART I/O library
//
//***************************************************************
/////////////////////////////////////////////////////////////////

#include "uartio.h"

//************************************
//  helper functions
//**************************************


unsigned int get_char_buffer_size(char* s)
{
    char *t;
    for (t = s; *t != '\0'; t++);
    return t - s;
}



///////////////////////////////////////////
//*****************************************
//  UART functions
//*****************************************
///////////////////////////////////////////

void UART_init()
{
    if (CALBC1_1MHZ==0xFF)                    // If calibration constant erased
    {
      while(1);                               // do not load, trap CPU!!
    }
    DCOCTL = 0;                               // Select lowest DCOx and MODx settings
    BCSCTL1 = CALBC1_1MHZ;                    // Set DCO
    DCOCTL = CALDCO_1MHZ;
    P1SEL |= BIT1 + BIT2;                     // P1.1 = RXD, P1.2=TXD
    P1SEL2 |= BIT1 + BIT2;                    // P1.1 = RXD, P1.2=TXD
    UCA0CTL1 |= UCSSEL_2;                     // SMCLK
    UCA0BR0 = 104;                            // 1MHz 9600
    UCA0BR1 = 0;                              // 1MHz 9600
    UCA0MCTL = UCBRS0;                        // Modulation UCBRSx = 1
    UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
}


int UART_init_new(
        unsigned char RXD_mask,     // 8-bit mask to enable RXD pin
        unsigned char TXD_mask      // 8-bit mask to enable TXD pin
        )
{
    // If calibration constant erased
    if (CALBC1_1MHZ==0xFF) return 1;

    DCOCTL = 0;                               // Select lowest DCOx and MODx settings
    BCSCTL1 = CALBC1_1MHZ;                    // Set DCO
    DCOCTL = CALDCO_1MHZ;

    // enable RXD and TXD pins
    P1SEL |= RXD_mask + TXD_mask;
    P1SEL2 |= RXD_mask + TXD_mask;

    UCA0CTL1 |= UCSSEL_2;                     // SMCLK
    UCA0BR0 = 104;                            // 1MHz 9600
    UCA0BR1 = 0;                              // 1MHz 9600
    UCA0MCTL = UCBRS0;                        // Modulation UCBRSx = 1
    UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**

    return 0;
}


void UART_putc(char c)
{
    while(!(IFG2 & UCA0TXIFG));
    UCA0TXBUF = c;
}

void UART_puts(char* s)
{
    unsigned int size = get_char_buffer_size(s);

    if (size==0) return;

    unsigned int i = 0;
    for(;i<size+1;i++)
        UART_putc(s[i]);
}

void UART_puts_new(char* s)
{
    if (get_char_buffer_size(s)==0) return;

    while(s!='\0')
        UART_putc(s++);
}

//////////////////////////////////////////////////////
//
// (TBD) new implementation for UART print:
//
/////////////////////////////////////////////////////
//
// putc puts a char in a TX_buffer, that will be
// sent over UART when the A0TXIFG is set. the
// A0TXIFG ISR must keep track of the A0TXIE bit
// and allocation of the TX_buffer
//
//////////////////////////////////////////////////////



