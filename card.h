#ifndef CARD_H
#define CARD_H

#include <RFID.h>
#include "util.h"

#define RST_PIN             A0
#define SDA_PIN             10
#define BEEPER              A2


#define DATA_LENGTH         16

#define MARK_SECTOR         15
#define APP_SECTOR          14
#define DATA_BLOCK_INDEX     2

#define ACTION_READ        'R'
#define ACTION_WRITE       'W'

struct _position {
    int sector;
    int block;

    int password_offset () {
        return sector * 4 + 3;
    };
    int data_offset () {
        return sector * 4 + block;
    };
};

class Card {
    private :
        RFID *rfid;
        unsigned char SN[5];
        struct _position mark, app;

        int read_sector (struct _position *pos, unsigned char* key, unsigned char *buff);
    public :
        Card (RFID *rfid);
        int readCard (unsigned char *buff);
        unsigned char DEFAULT_KEY[16] = {
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
            0xff, 0x07, 0x80, 0x69,
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
        };
        unsigned char APP_KEY[16] = {
            0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC,
            0xff, 0x07, 0x80, 0x69,
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
        };
};

Card::Card (RFID *rfid) {
    this->rfid = rfid;
    SPI.begin();

    pinMode (BEEPER, OUTPUT);
    digitalWrite (BEEPER, LOW);

    mark.sector = MARK_SECTOR;
    app.sector  = APP_SECTOR;
    app.block   = mark.block = DATA_BLOCK_INDEX;
}

/**
 * read block data at position:pos, with the key.
 * if read success, write the data to buff.
 */
int Card::read_sector (struct _position *pos, unsigned char *key, unsigned char *buff) {
    rfid -> init ();
    rfid -> isCard ();
    if (rfid -> readCardSerial ()) {
        memcpy (SN, rfid->serNum, 5);
    }
    rfid -> selectTag (SN);
    int state = rfid->auth (PICC_AUTHENT1A, pos -> password_offset (), key, SN);
    if (state == MI_OK) {
        return rfid -> read (pos -> data_offset (), buff);
    }
}

/**
 * read the card info
 */
int Card::readCard (unsigned char *buff) {
    int state = read_sector (&mark, DEFAULT_KEY, buff);
    if (state != MI_OK) {
        return state;
    }
    
    return read_sector (&app, DEFAULT_KEY, buff + 16);
}
#endif // CARD_H
