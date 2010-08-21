#include <avr/io.h>
#include <avr/interrupt.h>

#include "interface.h"
#include "turing.h"

void main(void) __attribute__ ((noreturn)); 

void main(void){
    uint8_t i;
    uint8_t disp[5];
    interface_setup();
    disp[1] = 0x00;
    tape_clear();
    //tape_putint(44511); //fib seq
    for(i = 0;;i++){
        
        disp[0] = btnState;
        //if(i % 10 == 0)
        disp[2] = btnPress;
        if(btn_read() & 0x02)    
            disp[1] ^= 0xff;
        //disp[2] = btnPress;
        //shift_data(disp, 3);
        //disp_tape();
        shift_data(tape, 3);
        disp_7seg((read_pot() & 0xff));
        
        
        delay_ms_poll(50);
        //disp7Seg((millis() >> 5) & 0xff);
        
    }
}
