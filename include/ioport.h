/* 
 * File:   ioport.h
 * Author: Julien Cassette
 *
 * Created on 26 juin 2014, 15:33
 */

#ifndef IOPORT_H
#define	IOPORT_H

#include <xc.h>             /* XC8 General Include File */
#include <stdint.h>         /* For uint8_t definition */
#include <stdbool.h>        /* For true/false definition */

#define PIN_0   (1 << 0)
#define PIN_1   (1 << 1)
#define PIN_2   (1 << 2)
#define PIN_3   (1 << 3)
#define PIN_4   (1 << 4)
#define PIN_5   (1 << 5)
#define PIN_6   (1 << 6)
#define PIN_7   (1 << 7)
#define PIN_ALL (PIN_0|PIN_1|PIN_2|PIN_3|PIN_4|PIN_5|PIN_6|PIN_7)

#define PortA_SetDirectionIn(pins)      TRISA |= pins
#define PortA_SetDirectionOut(pins)     TRISA &= ~pins
#define PortA_SetModeAnalog(pins)       ANCON0 |= pins
#define PortA_SetModeDigital(pins)      ANCON0 &= ~pins

#define PortB_SetDirectionIn(pins)      TRISB |= pins
#define PortB_SetDirectionOut(pins)     TRISB &= ~pins
#define PortB_SetModeAnalog(pins)       ANCON1 |= pins
#define PortB_SetModeDigital(pins)      ANCON1 &= ~pins
#define PortB_EnablePullUp(pins)        WPUB |= pins
#define PortB_DisablePullUp(pins)       WPUB &= ~pins

#define PortC_SetDirectionIn(pins)      TRISC |= pins
#define PortC_SetDirectionOut(pins)     TRISC &= ~pins
#define PortC_EnableOpenDrain(pins)     ODCON |= pins
#define PortC_DisableOpenDrain(pins)    ODCON &= ~pins

#endif	/* IOPORT_H */