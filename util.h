#ifndef _UTIL_H_
#define _UTIL_H_

#include <Arduino.h>

inline void beep (int beeper) {
    for (int i = 0; i < 100; i ++) {
        digitalWrite (beeper, HIGH);
        delayMicroseconds (200);
        digitalWrite (beeper, LOW);
        delayMicroseconds (200);
    }
    delay (200);
}

inline void error (int beeper) {
    for (int i = 0; i < 3; i ++) {
        beep (beeper);
    }
}
#endif //_UTIL_H_
