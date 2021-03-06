#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <string.h>

#include "turing.h"
#include "interface.h"


//uint8_t states[6] = {0x83, 0x05, 0x01, 0x83, 0x03, 0x7F}; //busybeaver
void Turing::tape_clear(){
    for(int8_t i = 0; i < BTAPESIZE; i++){
        tape[i] = 0x00;
    }
}

void Turing::tape_putchar(uint8_t v){
    for(int8_t i = 0; i <= 8; i+=8){
        tape[(tpos + i)/8] = (v & (0xff << (i - tpos % 8))) >> (i - tpos % 8);
    }
}

uint8_t Turing::tape_getsym(){
    return !!(tape[tpos/8] & (1 << (tpos % 8)));
}

void Turing::tape_putsym(uint8_t v){
    if(v){
        tape[tpos/8] |= (1 << (tpos % 8));
    }else{
        tape[tpos/8] &= ~(1 << (tpos % 8));
    }
}

void Turing::tape_flipsym(){
    tape[tpos/8] ^= (1 << (tpos % 8));
}

void Turing::tape_move_up(){
    if(tpos > 0){ 
        tpos--; 
    } else { 
        tpos = TAPESIZE-1;
    }
}

void Turing::tape_move_dn(){
    tpos++;
    if(tpos >= TAPESIZE){
        tpos = 0;
    }
}

void Turing::disp_tape(){
    uint8_t vtape[LIGHTBARS];
    memset(vtape, 0x00, LIGHTBARS);
    // Start at tpos?
    for(uint8_t i = 0; i < LIGHTBARS * 8; i++){
    
        //vtape[i] = (tape[tpos/8 + i] << (tpos % 8)) | (tape[tpos/8 + i + 1] >> (8 - tpos % 8));
    
        uint8_t pos = (i - TPOS_OFFSET + tpos + TAPESIZE) % TAPESIZE;
        
        vtape[i/8] |= (!!(tape[pos/8] & (1 << (pos % 8)))) << (i % 8);
        
        //pos++;
        //pos %= BTAPESIZE;
        
        //vtape[i] |= (tape[pos] >> (8 - mod));
        
    }
    shift_data(vtape, LIGHTBARS);
}



void Turing::turing_step(){
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
        tape_move_up();
    }else{
        tape_move_dn();
    }
}

Turing::Turing(){
    state = 0;
    tpos = 0;
}
