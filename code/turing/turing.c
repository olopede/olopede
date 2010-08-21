#include <avr/io.h>
#include <avr/interrupt.h>

#include "turing.h"
#include "interface.h"

uint16_t tpos = 0;        // Start in center of tape

void tape_clear(){
    for(int8_t i = 0; i < BTAPESIZE; i++){
        tape[i] = 'U';
    }
}

void tape_putint(uint16_t v){
    for(int8_t i = 0; i <= 16; i+=8){
        tape[tpos/8] = (v & (0xff << (i - tpos % 8))) >> (i - tpos % 8);
    }
}

void disp_tape(){
    uint8_t vtape[LIGHTBARS*8];
    // Start at tpos?
    for(uint8_t i = 0; i < LIGHTBARS; i++){
        vtape[i] = (tape[tpos/8 + i] << (tpos % 8)) | (tape[tpos/8 + i + 1] >> (8 - tpos % 8));
    }
    shift_data(vtape, LIGHTBARS);
}
