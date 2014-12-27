#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <can.h>

/**********************************************************
 *      GESTION DES IDENTIFIANTS
 *********************************************************/

typedef struct {
    uint8_t volatile sidh;
    uint8_t volatile sidl;
    uint8_t volatile eidh;
    uint8_t volatile eidl;
} can_id_regs;

static void write_can_id(can_id id, can_id_regs * regs)
{
    if (id & CAN_EID_FLAG) {
        id &= CAN_EID_MASK;
        regs->sidh = id >> 21 & 0xFF;
        regs->sidl = id >> 13 & 0xE0 | id >> 16 & 0x07 | 0x08;
        regs->eidh = id >> 8 & 0xFF;
        regs->eidl = id & 0xFF;
    } else {
        id &= CAN_SID_MASK;
        regs->sidh = id >> 3 & 0xFF;
        regs->sidl = id << 5 & 0xE0;
    }
}

static void read_can_id(can_id * id, can_id_regs const * regs)
{
    if (regs->sidl & 0x08) {
        *id = (uint32_t)regs->sidh << 21;
        *id |= ((uint32_t)regs->sidl & 0xE0) << 13;
        *id |= ((uint32_t)regs->sidl & 0x07) << 16;
        *id |= (uint32_t)regs->eidh << 8;
        *id |= (uint32_t)regs->eidl;
    } else {
        *id = (uint32_t)regs->sidh << 3;
        *id |= ((uint32_t)regs->sidl & 0xE0) >> 5;
    }
}

/**********************************************************
 *      MASQUES ET FILTRES
 *********************************************************/

#define CAN_MASK_N 2
static can_id_regs * const can_mask[CAN_MASK_N] = {
    (can_id_regs *)&RXM0SIDH,
    (can_id_regs *)&RXM1SIDH,
};
static unsigned char can_mask_idx;

#define CAN_FILTER_N 16
static can_id_regs * const can_filter[CAN_FILTER_N] = {
    (can_id_regs *)&RXF0SIDH,
    (can_id_regs *)&RXF1SIDH,
    (can_id_regs *)&RXF2SIDH,
    (can_id_regs *)&RXF3SIDH,
    (can_id_regs *)&RXF4SIDH,
    (can_id_regs *)&RXF5SIDH,
    (can_id_regs *)&RXF6SIDH,
    (can_id_regs *)&RXF7SIDH,
    (can_id_regs *)&RXF8SIDH,
    (can_id_regs *)&RXF9SIDH,
    (can_id_regs *)&RXF10SIDH,
    (can_id_regs *)&RXF11SIDH,
    (can_id_regs *)&RXF12SIDH,
    (can_id_regs *)&RXF13SIDH,
    (can_id_regs *)&RXF14SIDH,
    (can_id_regs *)&RXF15SIDH,
};
static unsigned char can_filter_idx;

/**********************************************************
 *      BUFFERS
 *********************************************************/

typedef struct {
    uint8_t volatile con;
    can_id_regs id;
    uint8_t volatile dlc;
    uint8_t volatile d[8];
} can_msg_regs;

#define CAN_BUFFER_N 11
static can_msg_regs * const can_buffer[CAN_BUFFER_N] = {
    (can_msg_regs *)&RXB0CON,
    (can_msg_regs *)&RXB1CON,
    (can_msg_regs *)&B0CON,
    (can_msg_regs *)&B1CON,
    (can_msg_regs *)&B2CON,
    (can_msg_regs *)&B3CON,
    (can_msg_regs *)&B4CON,
    (can_msg_regs *)&B5CON,
    (can_msg_regs *)&TXB2CON,
    (can_msg_regs *)&TXB1CON,
    (can_msg_regs *)&TXB0CON,
};
static can_msg_regs * const * can_buffer_rx;
static unsigned char can_rx_size;
static can_msg_regs * const * can_buffer_tx;
static unsigned char can_tx_size;

#ifndef CAN_FIFO_SIZE
#define CAN_FIFO_SIZE 4
#endif

/**********************************************************
 *      INITIALISATION
 *********************************************************/

enum CAN_MODE {
    CAN_MODE_NORMAL      = 0b000,
    CAN_MODE_DISABLE     = 0b001,
    CAN_MODE_LOOPBACK    = 0b010,
    CAN_MODE_LISTEN      = 0b011,
    CAN_MODE_CONFIG      = 0b100,
};

static void can_set_mode(enum CAN_MODE mode)
{
    CANCONbits.REQOP = mode;
    while (CANSTATbits.OPMODE != mode);
}

void can_init_begin()
{
    /* Passer en mode configuration */
    can_set_mode(CAN_MODE_CONFIG);

    /* Utiliser le module ECAN en mode 2 (FIFO) */
    ECANCONbits.MDSEL = 2;

    /* Configurer la taille de la FIFO */
    BSEL0 = 0xFF << CAN_FIFO_SIZE;

    /* Initialiser les tableaux permettant d'acceder aux buffers */
    can_buffer_rx = &can_buffer[0];
    can_rx_size = CAN_FIFO_SIZE;
    can_buffer_tx = &can_buffer[CAN_FIFO_SIZE];
    can_tx_size = CAN_BUFFER_N - CAN_FIFO_SIZE;

    /* Desactiver tous les filtres */
    RXFCON0 = 0;
    RXFCON1 = 0;

    /* Initialiser les index pour les masques et les filtres */
    can_mask_idx = 0;
    can_filter_idx = 0;
}

void can_init_baudrate(unsigned long int sysfreq, unsigned long int baudrate)
{
    /* Chercher le nombre de quanta qui correspond le mieux */
    unsigned int const ratio = sysfreq / (2 * baudrate);
    unsigned char quanta = 8;
    for (unsigned char i = quanta + 1; i <= 20; i++) {
        if (ratio % i < ratio % quanta) {
            quanta = i;
        }
    }

    /* Definir la taille des segments */
    uint8_t brp, segpr, segph1, segph2;
    brp = ratio / quanta;
    segpr = min(quanta / 4 - 1, 8);
    segph1 = min(quanta / 2, 8);
    segph2 = quanta - 1 - segpr - segph1;

    /* Ecrire les registres */
    BRGCON1bits.BRP = brp - 1;
    BRGCON2bits.PRSEG = segpr - 1;
    BRGCON2bits.SEG1PH = segph1 - 1;
    BRGCON2bits.SEG2PHTS = 1; // permet d'utiliser SEG2PH
    BRGCON3bits.SEG2PH = segph2 - 1;
    BRGCON2bits.SAM = 0;
    BRGCON1bits.SJW = 0;
}

void can_init_filter(can_id mask, can_id filter)
{
    static can_id last_mask; /* masque utilise au dernier appel */

    if (can_filter_idx >= CAN_FILTER_N) {
        /* Plus aucun filtre disponible */
        return;
    }

    if (can_mask_idx == 0 || last_mask != mask) {
        /* Ajout du premier masque ou d'un nouveau masque */
        if (can_mask_idx >= CAN_MASK_N) {
            /* Plus aucun masque disponible */
            return;
        }
        /* Ajouter le masque et le memoriser */
        write_can_id(mask, can_mask[can_mask_idx]);
        can_mask_idx++;
        last_mask = mask;
    }

    /* Ajouter le filtre */
    write_can_id(filter, can_filter[can_filter_idx]);
    can_filter_idx++;

    /* Associer le filtre au masque correspondant (via MSEL*)
     * et activer le filtre (via RXFCON*)
     */
    switch (can_filter_idx - 1) {
    case 0:
        MSEL0bits.FIL0 = can_mask_idx - 1;
        RXFCON0bits.RXF0EN = 1;
        break;
    case 1:
        MSEL0bits.FIL1 = can_mask_idx - 1;
        RXFCON0bits.RXF1EN = 1;
        break;
    case 2:
        MSEL0bits.FIL2 = can_mask_idx - 1;
        RXFCON0bits.RXF2EN = 1;
        break;
    case 3:
        MSEL0bits.FIL3 = can_mask_idx - 1;
        RXFCON0bits.RXF3EN = 1;
        break;
    case 4:
        MSEL1bits.FIL4 = can_mask_idx - 1;
        RXFCON0bits.RXF4EN = 1;
        break;
    case 5:
        MSEL1bits.FIL5 = can_mask_idx - 1;
        RXFCON0bits.RXF5EN = 1;
        break;
    case 6:
        MSEL1bits.FIL6 = can_mask_idx - 1;
        RXFCON0bits.RXF6EN = 1;
        break;
    case 7:
        MSEL1bits.FIL7 = can_mask_idx - 1;
        RXFCON0bits.RXF7EN = 1;
        break;
    case 8:
        MSEL2bits.FIL8 = can_mask_idx - 1;
        RXFCON1bits.RXF8EN = 1;
        break;
    case 9:
        MSEL2bits.FIL9 = can_mask_idx - 1;
        RXFCON1bits.RXF9EN = 1;
        break;
    case 10:
        MSEL2bits.FIL10 = can_mask_idx - 1;
        RXFCON1bits.RXF10EN = 1;
        break;
    case 11:
        MSEL2bits.FIL11 = can_mask_idx - 1;
        RXFCON1bits.RXF11EN = 1;
        break;
    case 12:
        MSEL3bits.FIL12 = can_mask_idx - 1;
        RXFCON1bits.RXF12EN = 1;
        break;
    case 13:
        MSEL3bits.FIL13 = can_mask_idx - 1;
        RXFCON1bits.RXF13EN = 1;
        break;
    case 14:
        MSEL3bits.FIL14 = can_mask_idx - 1;
        RXFCON1bits.RXF14EN = 1;
        break;
    case 15:
        MSEL3bits.FIL15 = can_mask_idx - 1;
        RXFCON1bits.RXF15EN = 1;
        break;
    }
}

void can_init_end()
{
    can_set_mode(CAN_MODE_NORMAL);
}

/**********************************************************
 *      ENVOI ET RECEPTION
 *********************************************************/

unsigned char can_send(can_msg const * msg)
{
    /* Trouver un buffer libre */
    unsigned int i;
    can_msg_regs * regs;
    for (i = 0; i < can_tx_size; i++) {
        regs = can_buffer_tx[i];
        if ((regs->con & _TXB0CON_TXREQ_MASK) == 0) {
            break;
        }
    }
    if (i == can_tx_size) {
        return 0;
    }

    /* Copier l'identifiant */
    write_can_id(msg->id, &regs->id);

    /* Copier la longueur des donnees */
    if (msg->length > 8) {
        return 0;
    }
    regs->dlc = msg->length;

    /* Copier les donnees */
    memcpy(regs->d, msg->data, msg->length);

    /* Marquer le message pour envoi */
    regs->con |= _TXB0CON_TXREQ_MASK;

    return 1;
}

unsigned char can_receive(can_msg * msg)
{
    unsigned char i = CANCON & 0x0F; // CANCONbits.FP;
    can_msg_regs * regs = can_buffer_rx[i];

    if ((regs->con & _RXB0CON_RXFUL_MASK) == 0) {
        return 0;
    }

    /* Copier l'identifiant */
    read_can_id(&msg->id, &regs->id);

    /* Copier la longueur des donnees */
    msg->length = regs->dlc & _RXB0DLC_DLC_MASK;
    if (msg->length > 8) {
        return 0;
    }

    /* Copier les donnees */
    memcpy(msg->data, regs->d, msg->length);

    /* Marquer le message comme lu */
    regs->con &= ~_RXB0CON_RXFUL_MASK;

    return 1;
}
