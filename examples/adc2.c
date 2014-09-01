/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/

#if defined(__XC)
#include <xc.h>             /* XC8 General Include File */
#include <stdint.h>         /* For uint8_t definition */
#include <stdbool.h>        /* For true/false definition */
#endif

#include "board.h"
#include <plib/adc.h>

void main(void)
{
    int16_t moyenne;

    BoardInit();

    /* Configuration de l'ADC
     *  horloge: oscillateur RC interne
     *  résultat justifié à droite
     *  temps d'acquisition: 2*Tad (délai minimum entre 2 conversions)
     *  canal 4
     *  interruption activée
     *  tensions référence par défaut (Vref+ = AVdd / Vref- = AVss)
     */
    OpenADC(ADC_FOSC_RC|ADC_RIGHT_JUST|ADC_2_TAD, ADC_CH4|ADC_INT_ON, 0);

    moyenne = 0;
    for (uint8_t i = 0; i < 16; i++) {
        ConvertADC();               /* démarrage conversion */
        Sleep();                    /* mise en veille CPU */
                                    /* réveil par interruption ADC */
        moyenne += ReadADC();       /* lecture résultat */
    }
    moyenne /= 16;

    while (1);
}