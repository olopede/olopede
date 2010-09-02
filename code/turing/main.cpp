#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>


#include "interface.h"
#include "turing.h"

void main(void) __attribute__ ((noreturn)); 

void main(void){
    uint8_t i;
    Turing turing = Turing();
    interface_setup();
    turing.tape_clear();
    /*
    tape_putchar(0xAD); //0b10101101);
    tpos += 8;
    tape_putchar(0xDF); //0b11011111);
    tpos -= 8; //fib seq
    */
    
    
    //uint8_t busybeaver[6] = {0x83, 0x05, 0x01, 0x83, 0x03, 0x7F};
    uint8_t copier[10] = {0x3E, 0x82, 0x84, 0x83, 0x07, 0x85, 0x08, 0x07, 0x81, 0x09};
    //memcpy(states, busybeaver, 6);
    
    //turing.states = busybeaver;
    turing.states = copier;
    
    for(i = 0;;i++){
        //turing.disp_tape();
        //turing.tape[1] = 'U';
        //turing.tape[turing.tpos/8] ^= 1 << (turing.tpos % 8);
        //shift_data(turing.tape, 3);
        //delay_ms_poll(1);
        //turing.tape[turing.tpos/8] ^= 1 << (turing.tpos % 8);
        //shift_data(turing.tape, 3);
        //disp_7seg_digit(turing.state >> 1);
        turing.tape_flipsym();
        turing.disp_tape();
        delay_ms(5);
        turing.tape_flipsym();
        turing.disp_tape();

        if(btn_read() & BTN_CTR){
            //TAPE_MV_UP;
            turing.turing_step();
        }
        if(btn_read_cached() & BTN_DL){
            turing.tape_move_dn();
        }
        if(btn_read_cached() & BTN_DR){
            turing.tape_move_up();
        }
        if(btn_read_cached() & BTN_DU){
            turing.tape_flipsym();
        }
        if(btn_read_cached() & BTN_SR){
            turing.state = 0;
        }
        
        delay_ms(20);        
    }
}
