//////////////////////////////////////////////////////////////////////////////////
//*******************************************************************************
//
//   I2C Address Scan (src/main.c)
//
//   Brian Costantibo
//
//********************************************************************************
//
//   -   This code, for the MSP430G2553, acts as the controller on an I2C bus.
//   -   Scans all 7-bit peripheral addresses, storing each valid address
//       in a global array (int[] addys)
//
//********************************************************************************
//////////////////////////////////////////////////////////////////////////////////

#include <msp430.h>
#include <stdio.h>
#include <UART_IO_msp430g2553.h>

int addys[5];
unsigned int counter = 0;

void I2C_init();

int main(void)
{
    // stop watchdog timer
    WDTCTL = WDTPW + WDTHOLD;

    // initialize serial communication
    I2C_init();
    UART_init();

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
    // determine number of devices
    unsigned int dev_count = 0;
    unsigned int i;
    for(int i=0;i<sizeof(addys)/sizeof(addys[0]);i++)
    {
        if(addys[i]!=0)
            dev_count++;
    }

    // print number of devices
    char message0[20];                                           // define initial message
    sprintf(message0, "Found %d device(s):\n\r", dev_count);    // format initial message
    UART_puts(message0);                                        // print message through UART

    // print all device addresses
    for(i=0;i<dev_count;i++)
    {
        char message1[20];
        sprintf(message1, "Address: 0x%02X (%d)\n\r", addys[i], addys[i]);
        UART_puts(message1);
    }

    while(1);
}

//*****************************************************************
// Initialization function for I2C communication
//*****************************************************************

void I2C_init()
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

// I2C RX/TX ISR
// UART TX ISR
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCIAB0TX_VECTOR))) USCI0TX_ISR (void)
#else
#error Compiler not supported!
#endif
{
    // recieved a message over I2C
    if (IFG2 & UCB0RXIFG)
    {
        unsigned char rx_val = UCB0RXBUF; //Must read UCxxRXBUF to clear the flag
        addys[counter] = UCB0I2CSA;
        counter++;
    }

    // UART TX buff is empty
    /*
    if (IFG2 & UCA0TXIFG)
        UART_sendc();
    */

    // exit low power mode
    __bic_SR_register_on_exit(LPM0_bits);
}

