/////////////////////////////////////////////////////////////////
//***************************************************************
//
//  lib/UART_IO_msp430g2553.c
//  register level c code for MSP430G2553 UART I/O API
//
//***************************************************************
/////////////////////////////////////////////////////////////////

#include <UART_IO_msp430g2553.h>

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
    UCA0BR0 = BAUD_9600;                      // 1MHz 9600
    UCA0BR1 = 0;                              // 1MHz 9600
    UCA0MCTL = UCBRS0;                        // Modulation UCBRSx = 1
    UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**

    // initialize print queue
    queue_init(&UART_print_q);

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

/*      TEST CODE  UNUSED
char UART_TX_buffer[MAX_PRINT_BUFFER_LENGTH];
unsigned int UART_TX_add_pointer = 0,
        UART_TX_print_pointer = 0;

// to be called only when USCIA0TXIFG is set
void UART_SEND_CHAR()
{
    // reset print pointer to 0 if greater than TX buffer length
    if(UART_TX_print_pointer >= sizeof(UART_TX_buffer)/sizeof(UART_TX_buffer[0]))
        UART_TX_print_pointer = 0;

    // pull next byte to be sent, and increment print pointer
    char next_byte = UART_TX_buffer[UART_TX_print_pointer];
    UART_TX_buffer[UART_TX_print_pointer] = 69;
    UCA0TXBUF = next_byte;
}

void UART_putc_new(char c)
{
    // reset add pointer to 0 if greater than TX buffer length
    if((UART_TX_add_pointer >= sizeof(UART_TX_buffer)/sizeof(UART_TX_buffer[0])) ||
            (UART_TX_add_pointer >= UART_TX_print_pointer))
        UART_TX_add_pointer = 0;

    // add char to TX buffer and increment add pointer
    UART_TX_buffer[UART_TX_add_pointer++] = c;
}
*/

/////////////////////////////////////////////
//*******************************************
// CIRCULAR QUEUE   static allocation
//*******************************************
/////////////////////////////////////////////

queue_t UART_print_q;

int queue_init(queue_t* q)
{
    q->num_values=0;
    q->head=0;
    q->tail=0;
}

int enqueue(queue_t* q, char c)
{
    if(q->num_values==QUEUE_SIZE) return 1;

    q->values[q->tail]=c;
    q->num_values++;
    q->tail=(q->tail+1)%QUEUE_SIZE;

    return 0;
}

char dequeue(queue_t* q)
{
    if(q->num_values==0) return QUEUE_EMPTY;

    char c = q->values[q->head];
    q->head=(q->head+1)%QUEUE_SIZE;
    q->num_values--;

    return c;
}

void UART_putc_new(char c)
{
    enqueue(&UART_print_q, c);
    IE2 |= UCA0TXIE;
}

void UART_sendc()
{
    char nextc = dequeue(&UART_print_q);
    if(nextc==QUEUE_EMPTY)
    {
        IE2 &= ~UCA0TXIE;
        return;
    }

    UCA0TXBUF = nextc;
}
