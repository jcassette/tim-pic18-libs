#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include <ioport.h>

void BoardInit()
{
    /* DIP switch */
    PortA_SetDirectionIn(PIN_0|PIN_1|PIN_2|PIN_3);
    /* Potentiometre */
    PortA_SetDirectionIn(PIN_5);    /* RA5 = AN4 */
    PortA_SetModeAnalog(PIN_5);
    /* CAN */
    PortB_SetDirectionOut(PIN_2);   /* RB2 = CANTX */
    PortB_SetDirectionIn(PIN_3);    /* RB3 = CANRX */
    /* LED */
    PortC_SetDirectionOut(PIN_0|PIN_1|PIN_2|PIN_3);
    /* UART */
    PortC_SetDirectionOut(PIN_6);   /* RC6 = TX */
    PortC_SetDirectionIn(PIN_7);    /* RC7 = RX */
}