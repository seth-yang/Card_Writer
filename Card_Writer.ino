#include <debug.h>
#include "command.h"
#include "card.h"

RFID rfid (SDA_PIN, RST_PIN);
Card card (&rfid);
Command cmd;

void setup () {
    Serial.begin(9600);
}

void loop() {
    if (cmd.is_ready ()) {
        process_command ();
        cmd.clear_command ();
    }
}

/*########################## implementions ############################ */
void process_command () {
    if (cmd.check_command ()) {
        switch (cmd.command.data.action) {
            case ACTION_READ : {
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
                break;
            }
            case ACTION_WRITE :
                break;
            default :
                break;
        }
    }
}
