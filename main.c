/* --COPYRIGHT--,BSD_EX
 * Copyright (c) 2012, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *******************************************************************************
 * 
 *                       MSP430 CODE EXAMPLE DISCLAIMER
 *
 * MSP430 code examples are self-contained low-level programs that typically
 * demonstrate a single peripheral function or device feature in a highly
 * concise manner. For this the code may rely on the device's power-on default
 * register values and settings such as the clock configuration and care must
 * be taken when combining code from several examples to avoid potential side
 * effects. Also see www.ti.com/grace for a GUI- and www.ti.com/msp430ware
 * for an API functional library-approach to peripheral configuration.
 *
 * --/COPYRIGHT--*/
//******************************************************************************
//  MSP430G2xx3 Demo - USCI_B0 I2C Master RX single bytes from MSP430 Slave
//
//  Description: This demo connects two MSP430's via the I2C bus. The master
//  reads from the slave. This is the master code. The data from the slave
//  transmitter begins at 0 and increments with each transfer. The received
//  data is in R5 and is checked for validity. If the received data is
//  incorrect, the CPU is trapped and the P1.0 LED will stay on. The USCI_B0
//  RX interrupt is used to know when new data has been received.
//  ACLK = n/a, MCLK = SMCLK = BRCLK = default DCO = ~1.2MHz
//
//  *** to be used with "msp430g2xx3_uscib0_i2c_05.c" ***
//
//                                /|\  /|\
//               MSP430G2xx3      10k  10k     MSP430G2xx3
//                   slave         |    |        master
//             -----------------   |    |  -----------------
//           -|XIN  P1.7/UCB0SDA|<-|---+->|P1.7/UCB0SDA  XIN|-
//            |                 |  |      |                 | 32kHz
//           -|XOUT             |  |      |             XOUT|-
//            |     P1.6/UCB0SCL|<-+----->|P1.6/UCB0SCL     |
//            |                 |         |             P1.0|--> LED
//
//  D. Dang
//  Texas Instruments Inc.
//  February 2011
//   Built with CCS Version 4.2.0 and IAR Embedded Workbench Version: 5.10
//******************************************************************************
#include <msp430.h>
#include <stdio.h>

int addys[10];
unsigned int counter = 0;

void initializeI2C()
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
    //UCB0I2CIE = UCNACKIE;
}

void initUART()
{
    if (CALBC1_1MHZ==0xFF)                    // If calibration constant erased
    {
      while(1);                               // do not load, trap CPU!!
    }
    UCA0CTL1 |= UCSWRST;
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

int main(void)
{
    WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
    P1DIR |= BIT0;                            // P1.0 output
    P1OUT &= BIT0;                            // P1.0 = 1

    initializeI2C();
    initUART();

    UCB0I2CSA = 0x00;                         // start address scan at 0x00, slave address
    while (1)
    {
        while (UCB0CTL1 & UCTXSTP);              // delay while stop condition is set
        UCB0CTL1 |= UCTXSTT;                     // set I2C start condition

        while (UCB0CTL1 & UCTXSTT);              // Start condition sent?
        UCB0CTL1 |= UCTXSTP;                     // I2C stop condition

        if(!(UCB0STAT & UCNACKIFG))
        {
            __bis_SR_register(LPM0_bits+GIE);                  // Enter LPM0 w/ interrupts
            __no_operation();
        }

        if(UCB0I2CSA >= 0x7F)
            break;
        else
            UCB0I2CSA++;
    }

    while(1);

    // Uart stuff
    int i;
    for(i = 2; i<8;i++)
    {
        IE2 |= UCA0TXIE;
        UCA0TXBUF = i;                       // Enable USCI_A0 RX interrupt
        __bis_SR_register(LPM0_bits+GIE);
    }

    return 0;
}

// status interrupt handler
// STT, STP, NACK, etc.
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCIAB0RX_VECTOR))) USCI0RX_ISR (void)
#else
#error Compiler not supported!
#endif
{
    if(UCB0STAT & UCNACKIFG) // NACK
    {
        // do nothing
        UCB0STAT &= ~UCNACKIFG;
    }

    /*
    if (IFG2 & UCA0RXIFG)
    {
        int rx_val = UCA0RXBUF; //Must read UCxxRXBUF to clear the flag
    }
    */
}

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

