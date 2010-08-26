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
    
    
    uint8_t busybeaver[6] = {0x83, 0x05, 0x01, 0x83, 0x03, 0x7F};
    //memcpy(states, busybeaver, 6);
    
    turing.states = busybeaver;
    for(i = 0;;i++){
        //disp_tape();
        //tape[tpos/8] ^= 1 << (tpos % 8);
        //shift_data(tape, 3);
        delay_ms_poll(1);
        //tape[tpos/8] ^= 1 << (tpos % 8);
        //shift_data(tape, 3);
        //disp_7seg_digit(state >> 1);
        
        if(btn_read() & 0x02){
            //TAPE_MV_UP;
            turing.turing_step();
        }
        
        delay_ms_poll(20);        
    }
}
