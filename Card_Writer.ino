#include <debug.h>
#include "command.h"
#include "card.h"

#define ACTION_READ        'R'
#define ACTION_WRITE       'W'
#define ACTION_ERROR_CODE  'C'

RFID rfid (SDA_PIN, RST_PIN);
Card card (&rfid);
Command cmd;

void setup () {
    Serial.begin(115200);
    pinMode (13, OUTPUT);
    pinMode (BEEPER, OUTPUT);
    digitalWrite (BEEPER, LOW);
}

void loop() {
    if (cmd.is_ready ()) {
        process_command ();
        cmd.clear_command ();
    }
}

/*########################## implementions ############################ */
void read_card () {
    unsigned char buff[32];
    memset (buff, 0, 32);
    int state = card.read (buff);
    if (state == MI_OK) {
        int sum = 33 + ACTION_READ;
        for (int i = 0; i < 32; i ++) sum += buff [i] & 0xff;
        sum &= 0xff;
        Serial.write (0xca);             // header 0
        Serial.write (0xfe);             // header 1
        Serial.write (1);                // state = success
        Serial.write (ACTION_READ);      // type = read card
        Serial.write (32);               // length
        Serial.write (buff, 32);         // data
        Serial.write (sum);              // CRC
        Serial.write (0xba);             // tail
        Serial.write (0xbe);

        beep (BEEPER);
    } else {
        responseError (0, ACTION_READ, state);
        error (BEEPER);
    }
}

void write_card (struct CNG_DATA *data) {
    unsigned char buff[9] = {
        0xca, 0xfe, 
        0,                    // index = 2, state
        ACTION_ERROR_CODE,    // index = 3, type, 'E' = Error Code
        1,                    // index = 4, length, in this case, just one byte code.
        0,                    // index = 5, the code value
        0,                    // index = 6, CRC
        0xba, 0xbe            // tail
    };
    int state = card.write (data);
    if (state == MI_OK) {
        beep (BEEPER);
    } else {
        error (BEEPER);
    }
    responseError (state == MI_OK, ACTION_WRITE, state);
}

void responseError (char state, char action, char code) {
    unsigned char buff[9] = {
        0xca, 0xfe, 
        0,                    // index = 2, state, zero for fail
        0,                    // index = 3, action
        1,                    // index = 4, length, in this case, just one byte code.
        0,                    // index = 5, the code value
        0,                    // index = 6, CRC
        0xba, 0xbe            // tail
    };
    buff[2] = state;
    buff[3] = action;
    buff[5] = code;
    int sum = 0;
    for (int i = 2; i < 6; i ++) {
        sum += buff[i] & 0xff;
    }
    sum &= 0xff;
    buff[6] = sum;
    Serial.write (buff, 9);
    Serial.flush ();
}

void process_command () {
    if (cmd.check_command ()) {
        switch (cmd.command.data.action) {
            case ACTION_READ :
                read_card ();
                break;
            case ACTION_WRITE : 
                write_card (&(cmd.command.data));
                break;
            default :
                break;
        }
    }
}
