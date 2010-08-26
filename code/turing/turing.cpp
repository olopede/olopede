#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "turing.h"
#include "interface.h"


//uint8_t states[6] = {0x83, 0x05, 0x01, 0x83, 0x03, 0x7F}; //busybeaver
void tape_clear(struct Turing *turing){
    for(int8_t i = 0; i < BTAPESIZE; i++){
        turing->tape[i] = 0x00;
    }
}

void tape_putchar(struct Turing *turing, uint8_t v){
    for(int8_t i = 0; i <= 8; i+=8){
        turing->tape[(turing->tpos + i)/8] = (v & (0xff << (i - turing->tpos % 8))) >> (i - turing->tpos % 8);
    }
}

uint8_t tape_getsym(struct Turing *turing){
    return turing->tape[turing->tpos/8] & (1 << (turing->tpos % 8)) ? 1 : 0;
}

void tape_putsym(struct Turing *turing, uint8_t v){
    if(v){
        turing->tape[turing->tpos/8] |= (1 << (turing->tpos % 8));
    }else{
        turing->tape[turing->tpos/8] &= ~(1 << (turing->tpos % 8));
    }
}

void tape_move_up(struct Turing *turing){
    if(turing->tpos > 0){ 
        turing->tpos--; 
    } else { 
        turing->tpos = TAPESIZE-1;
    }
}

void tape_move_dn(struct Turing *turing){
    turing->tpos++;
    if(turing->tpos >= TAPESIZE){
        turing->tpos = 0;
    }
}

void disp_tape(struct Turing *turing){
    uint8_t vtape[LIGHTBARS];
    // Start at tpos?
    for(uint8_t i = 0; i < LIGHTBARS; i++){
    
        //vtape[i] = (tape[tpos/8 + i] << (tpos % 8)) | (tape[tpos/8 + i + 1] >> (8 - tpos % 8));
    
        int16_t pos = turing->tpos/8 + i - TPOS_OFFSET/8;
        uint8_t mod = (turing->tpos + TPOS_OFFSET) % 8;
        
        if(pos < 0){
            pos += BTAPESIZE;
        }else if(pos >= BTAPESIZE){
            pos -= BTAPESIZE;
        }
        vtape[i] = (turing->tape[pos] << (mod));
        
        pos++;
        
        if(pos < 0){
            pos += BTAPESIZE;
        }else if(pos >= BTAPESIZE){
            pos -= BTAPESIZE;
        }
        vtape[i] |= (turing->tape[pos] >> (8 - mod));
        
    }
    shift_data(vtape, LIGHTBARS);
}



void turing_step(struct Turing *turing){
    if(turing->state == TURING_STATE_HALT){
        return;     // HALT state!
    }
    uint8_t statedata = turing->states[turing->state | tape_getsym(turing)];
    turing->state = statedata & TURING_STATE_MASK;
    #ifdef TURING_NOP_MASK
    if(statedata & TURING_NOP_MASK){
        return;     // Don't replace the symbol
    }
    #endif
    tape_putsym(turing, statedata & TURING_SYM_MASK);
    if(statedata & TURING_MV_MASK){
        tape_move_up(turing);
    }else{
        tape_move_dn(turing);
    }
}
