#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "turing.h"
#include "interface.h"

uint16_t tpos = TAPESIZE/2;        // Start in center of tape
uint8_t state = 0;                 // Initial state
//uint8_t states[6] = {0x83, 0x05, 0x01, 0x83, 0x03, 0x7F}; //busybeaver
void tape_clear(){
    for(int8_t i = 0; i < BTAPESIZE; i++){
        tape[i] = 0x00;
    }
}

void tape_putchar(uint8_t v){
    for(int8_t i = 0; i <= 8; i+=8){
        tape[(tpos + i)/8] = (v & (0xff << (i - tpos % 8))) >> (i - tpos % 8);
    }
}

uint8_t tape_getsym(){
    return tape[tpos/8] & (1 << (tpos % 8)) ? 1 : 0;
}

void tape_putsym(uint8_t v){
    if(v){
        tape[tpos/8] |= (1 << (tpos % 8));
    }else{
        tape[tpos/8] &= ~(1 << (tpos % 8));
    }
}


void disp_tape(){
    uint8_t vtape[LIGHTBARS];
    // Start at tpos?
    for(uint8_t i = 0; i < LIGHTBARS; i++){
    
        //vtape[i] = (tape[tpos/8 + i] << (tpos % 8)) | (tape[tpos/8 + i + 1] >> (8 - tpos % 8));
    
        int16_t pos = tpos/8 + i - TPOS_OFFSET/8;
        uint8_t mod = (tpos + TPOS_OFFSET) % 8;
        
        if(pos < 0){
            pos += BTAPESIZE;
        }else if(pos >= BTAPESIZE){
            pos -= BTAPESIZE;
        }
        vtape[i] = (tape[pos] << (mod));
        
        pos++;
        
        if(pos < 0){
            pos += BTAPESIZE;
        }else if(pos >= BTAPESIZE){
            pos -= BTAPESIZE;
        }
        vtape[i] |= (tape[pos] >> (8 - mod));
        
    }
    shift_data(vtape, LIGHTBARS);
}

void turing_step(void){
    if(state == TURING_STATE_HALT){
        return;     // HALT state!
    }
    uint8_t statedata = states[state | tape_getsym()];
    state = statedata & TURING_STATE_MASK;
    #ifdef TURING_NOP_MASK
    if(statedata & TURING_NOP_MASK){
        return;     // Don't replace the symbol
    }
    #endif
    tape_putsym(statedata & TURING_SYM_MASK);
    if(statedata & TURING_MV_MASK){
        TAPE_MV_UP;
    }else{
        TAPE_MV_DN;
    }
}
