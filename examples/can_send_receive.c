#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

#include <ioport.h>
#include <can.h>

#include "board.h"

void main()
{
    BoardInit();

    can_init_begin();
    can_init_baudrate(8000000, 100000);
    can_init_filter(0x7FC, 0x010); // accepter 0x010, 0x011, 0x012, 0x013
    can_init_filter(0x7FC, 0x020); // accepter 0x020, 0x021, 0x022, 0x023
    can_init_filter(0x7FF, 0x030); // accepter 0x030
    can_init_end();

    uint8_t nb_msg = 0;
    while (1) {
        bool envoyer_nb_msg = false;

        can_msg msg_recu;
        if (can_receive(&msg_recu)) {
            nb_msg++;
            if (msg_recu.id == 0x030) {
                envoyer_nb_msg = true;
            } else if ((msg_recu.id & 0x7FC) == 0x010) {
                // allumer une LED
                unsigned char nled = msg_recu.id & 0x003;
                PORTC |= (1 << nled);
            } else if ((msg_recu.id & 0x7FC) == 0x020) {
                // eteindre une LED
                unsigned char nled = msg_recu.id & 0x003;
                PORTC &= ~(1 << nled);
            }
        }

        if (envoyer_nb_msg) {
            // envoyer le nombre de messages recus
            can_msg msg_envoi;
            msg_envoi.id = 0x031;
            msg_envoi.data[0] = nb_msg;
            msg_envoi.length = 1;
            while (! can_send(&msg_envoi));
        }
    }
}
