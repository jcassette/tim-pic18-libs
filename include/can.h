/*
 * File:   can.h
 * Author: Julien Cassette
 *
 */

#ifndef CAN_H
#define CAN_H

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

typedef uint32_t can_id;
#define CAN_EID_FLAG 0x80000000UL
#define CAN_EID_MASK 0x1FFFFFFFUL
#define CAN_SID_MASK 0x000007FFUL

typedef struct {
    can_id      id;
    uint8_t     data[8];
    uint8_t     length;
} can_msg;

void can_init_begin();
void can_init_baudrate(unsigned long int sysfreq, unsigned long int baudrate);
void can_init_filter(can_id mask_id, can_id filt_id);
void can_init_end();
unsigned char can_send(can_msg const * msg);
unsigned char can_receive(can_msg * msg);

#endif	/* CAN_H */
