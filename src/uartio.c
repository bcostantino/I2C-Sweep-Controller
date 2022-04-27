#include <msp430.h>
#include <uartio.h>

void get_char_buffer_size(unsigned int* size, char* s)
{
    char *t;
    for (t = s; *t != '\0'; t++);
    *size = t - s;
}

void uart_puts(char* s)
{
    unsigned int size;
    get_char_buffer_size(&size, s);

    if (size==0) return;

    unsigned int i = 0;
    for(;i<size+1;i++)
        uart_putc(s[i]);
}

void uart_putc(char c)
{
    while(!(IFG2 & UCA0TXIFG));
    UCA0TXBUF = c;
}



