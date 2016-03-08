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
        struct _position mark, app;

        int read_sector (struct _position *pos, unsigned char *key, unsigned char *buff);
        int write_sector (struct _position *pos, unsigned char *key, unsigned char *data);
    public :
        Card (RFID *rfid);
        int readCard (unsigned char *buff);
        int writeMark (uint16_t index, uint32_t timestamp, uint8_t mainVersion, uint8_t minVersion);
        int writeData (uint8_t isAdmin, uint32_t expire, uint32_t userId);
        
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
    unsigned char SN[5];
    memset (SN, 0, 5);
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
 * write the data to block as pos with key
 */
int Card::write_sector (struct _position *pos, unsigned char *key, unsigned char *data) {
    rfid -> init ();
    rfid -> isCard ();
    unsigned char SN[5];
    memset (SN, 0, 5);
    if (rfid -> readCardSerial ()) {
        memcpy (SN, rfid->serNum, 5);
    }
    rfid -> selectTag (SN);
    int state = rfid->auth (PICC_AUTHENT1A, pos -> password_offset (), key, SN);
    if (state == MI_OK) {
        return rfid -> write (pos -> data_offset (), data);
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

/**
 * 
 */
int Card::writeMark (uint16_t index, uint32_t timestamp, uint8_t mainVersion, uint8_t minVersion) {
    unsigned char buff[16] = {0xca, 0xfe, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xba, 0xbe};
    // index
    buff [2] = (index >> 8) & 0xff;
    buff [3] = index & 0xff;

    // timestamp
    buff [4] = (timestamp >> 24) & 0xff;
    buff [5] = (timestamp >> 16) & 0xff;
    buff [6] = (timestamp >>  8) & 0xff;
    buff [7] = timestamp         & 0xff;

    // main version
    buff [8] = mainVersion;
    buff [9] = minVersion;

    return write_sector (&mark, DEFAULT_KEY, buff);
}

/**
 * 
 */
int Card::writeData (uint8_t isAdmin, uint32_t expire, uint32_t userId) {
    unsigned char buff[16] = {0xca, 0xfe, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xba, 0xbe};

    // admin
    buff [2] = isAdmin;

    // expire
    buff [3] = (expire >> 24) & 0xff;
    buff [4] = (expire >> 16) & 0xff;
    buff [5] = (expire >>  8) & 0xff;
    buff [6] = expire         & 0xff;

    // user id
    buff [ 7] = (userId >> 24) & 0xff;
    buff [ 8] = (userId >> 16) & 0xff;
    buff [ 9] = (userId >>  8) & 0xff;
    buff [10] = userId         & 0xff;

    return write_sector (&app, DEFAULT_KEY, buff);
}
#endif // CARD_H
