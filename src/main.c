//*******************************************************************************
//
//   I2C Address Scan (src/main.c)
//
//   Brian Costantibo
//
//*********************************************************************************
//
//   -   This code, for the MSP430G2553, acts as the controller on an I2C bus.
//   -   Scans all 7-bit peripheral addresses, storing each valid address
//       in a global array (int[] addys)
//
//*********************************************************************************

#include <msp430.h>
#include <stdio.h>

#include "uartio.h"

int addys[10];
unsigned int counter = 0;

void initUART();
void initI2C();

int main(void)
{
    // stop watchdog timer
    WDTCTL = WDTPW + WDTHOLD;

    // initialize serial communication
    initI2C();
    initUART();

    // start address scan at peripheral address 0x00
    UCB0I2CSA = 0x00;

    while (1)
    {
        // transmit start condition over I2C bus
        while (UCB0CTL1 & UCTXSTP);              // delay while stop condition is set
        UCB0CTL1 |= UCTXSTT;                     // set I2C start condition
        while (UCB0CTL1 & UCTXSTT);              // Start condition sent?
        UCB0CTL1 |= UCTXSTP;                     // I2C stop condition

        // enter if request acknowledged
        if(!(UCB0STAT & UCNACKIFG))
        {
            // Enter LPM0 w/ interrupts
            __bis_SR_register(LPM0_bits+GIE);
            __no_operation();
        }

        // increment address, and break loop if greater than 7-bits
        if(UCB0I2CSA >= 0x7F)
            break;
        else
            UCB0I2CSA++;
    }

    // UART stuff
    char message[20];
    sprintf(message, "Device address: %d", addys[0]);
    uart_puts(message);

    while(1);
}

//*****************************************************************
//
// Initialization functions for I2C and UART communication
//
//******************************************************************8

void initI2C()
{
    UCB0CTL1 |= UCSWRST;                      // Enable SW reset

    P1SEL |= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0
    P1SEL2|= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0

    UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
    UCB0CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
    UCB0BR0 = 24;                             // set baud rate, fSCL = SMCLK/12 = ~100kHz
    UCB0BR1 = 0;
    UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
    IE2 |= UCB0RXIE + UCB0TXIE;
}

void initUART()
{
    if (CALBC1_1MHZ==0xFF)                    // If calibration constant erased
    {
      while(1);                               // do not load, trap CPU!!
    }
    DCOCTL = 0;                               // Select lowest DCOx and MODx settings
    BCSCTL1 = CALBC1_1MHZ;                    // Set DCO
    DCOCTL = CALDCO_1MHZ;
    P1SEL |= BIT1 + BIT2 ;                     // P1.1 = RXD, P1.2=TXD
    P1SEL2 |= BIT1 + BIT2 ;                    // P1.1 = RXD, P1.2=TXD
    UCA0CTL1 |= UCSSEL_2;                     // SMCLK
    UCA0BR0 = 104;                            // 1MHz 9600
    UCA0BR1 = 0;                              // 1MHz 9600
    UCA0MCTL = UCBRS0;                        // Modulation UCBRSx = 1
    UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
}

//*************************************************
//
// Interrupt Service Routines
//
//************************************************

// status interrupt handler (for STT, STP, NACK, etc.)
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCIAB0RX_VECTOR))) USCI0RX_ISR (void)
#else
#error Compiler not supported!
#endif
{
}

// RX/TX interrupt handler
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCIAB0TX_VECTOR))) USCI0TX_ISR (void)
#else
#error Compiler not supported!
#endif
{
    if (IFG2 & UCB0RXIFG)
    {
        unsigned char rx_val = UCB0RXBUF; //Must read UCxxRXBUF to clear the flag
        addys[counter] = UCB0I2CSA;
        counter++;
    }
    __bic_SR_register_on_exit(LPM0_bits);
}

