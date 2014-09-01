#if defined(__XC)
#include <xc.h>             /* XC8 General Include File */
#include <stdint.h>         /* For uint8_t definition */
#include <stdbool.h>        /* For true/false definition */
#endif

#include <ioport.h>

#pragma config FOSC	= INTIO2
#pragma config SOSCSEL	= DIG
#pragma config MCLRE	= OFF
#pragma config XINST    = OFF
#pragma config BOREN    = OFF
#pragma config WDTEN    = OFF

void BoardInit(void)
{
    /* DIP switch */
    PortA_SetDirectionIn(PIN_0|PIN_1|PIN_2|PIN_3);
    /* potentiometer */
    PortA_SetDirectionIn(PIN_5);
    PortA_SetModeAnalog(PIN_5);     /* RA5 is AN4 */
    /* CAN */
    PortB_SetDirectionOut(PIN_2);   /* RB2 is CANTX */
    PortB_SetDirectionIn(PIN_3);    /* RB3 is CANRX */
    /* LEDs */
    PortC_SetDirectionOut(PIN_0|PIN_1|PIN_2|PIN_3);
    /* UART */
    PortC_SetDirectionOut(PIN_6);   /* RC6 is TX */
    PortC_SetDirectionIn(PIN_7);    /* RC7 is RX */
}