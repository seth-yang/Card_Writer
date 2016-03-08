#include <debug.h>
#include "command.h"
#include "card.h"

#define ACTION_READ        'R'
#define ACTION_WRITE       'W'

RFID rfid (SDA_PIN, RST_PIN);
Card card (&rfid);
Command cmd;

void setup () {
    Serial.begin(9600);

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
    if (card.readCard (buff) == MI_OK) {
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
        error (BEEPER);
    }
}

void write_card (struct CNG_DATA *data) {
    int state = card.writeMark (
        data -> id, 
        data -> timestamp, 
        data -> main_version, 
        data -> min_version
    );
    
    if (state == MI_OK) {
        if (card.writeData (
                data -> isAdmin, 
                data ->expire, 
                data -> cardNo
            ) == MI_OK) {
            beep (BEEPER);
            return;
        }
    }
    error (BEEPER);
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
