/////////////////////////////////////////////////////////////////
//***************************************************************
//
//  lib/UART_IO_msp430g2553.h
//  c header file for MSP430G2553 UART I/O API
//
//***************************************************************
/////////////////////////////////////////////////////////////////


#include <msp430.h>


////////////////////////////////////////
//**************************************
//  tools and helper declarations
//**************************************
////////////////////////////////////////


unsigned int get_char_buffer_size(char* s);


/////////////////////////////////////////////////////////////////
//***************************************************************
//  circular queue
//***************************************************************
/////////////////////////////////////////////////////////////////
//***************************************************************
//  queue is sized statically, defined by QUEUE_SIZE macro
//  For more info on a similar implementation, see:
//      https://www.youtube.com/watch?v=oyX30WVuEos
//***************************************************************
/////////////////////////////////////////////////////////////////


// define constants
#define QUEUE_SIZE      20              // size for values array
#define QUEUE_EMPTY     '\0'            // character to represent empty queue response from dequeue

// define queue type
typedef struct queue
{
    char values[QUEUE_SIZE];
    unsigned int head, tail, num_values;
} queue_t;

// declare print queue
extern queue_t UART_print_q;

// declare queue manipulation functions
int queue_init(queue_t* q);             // initialize queue
int enqueue(queue_t* q, char c);        // set value at tail of queue
char dequeue(queue_t* q);               // get value from head of queue


///////////////////////////////////////////
//*****************************************
//  UART declarations
//*****************************************
///////////////////////////////////////////


#define BAUD_9600       104


void UART_init();


//*****************************************
//  UART TX
//*****************************************


void UART_putc(char c);
void UART_puts(char* s);


void UART_SEND_CHAR();
