#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>


#include "interface.h"
#include "turing.h"


Turing turing;

void _btn_ctr_press(){
    turing.turing_step();
}

void _btn_sl_hold(){
    turing.tape_putsym(0);
}

void _btn_sr_hold(){
    turing.tape_putsym(1);
}

void _btn_dl_hold(){
    turing.tape_move_up();
}

void _btn_dr_hold(){
    turing.tape_move_dn();
}

void main(void) __attribute__ ((noreturn)); 

void main(void){
    uint8_t i;
    turing = Turing();
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
    
    //turing.states = busybeaver;
    turing.states = copier;
    
    btn_ctr_press = &_btn_ctr_press;
    btn_sl_hold = &_btn_sl_hold;
    btn_sr_hold = &_btn_sr_hold;
    btn_dl_hold = &_btn_dl_hold;
    btn_dr_hold = &_btn_dr_hold;
    
    for(i = 0;;i++){
        //turing.disp_tape();
        //turing.tape[1] = 'U';
        //turing.tape[turing.tpos/8] ^= 1 << (turing.tpos % 8);
        //shift_data(turing.tape, 3);
        //delay_ms_poll(1);
        //turing.tape[turing.tpos/8] ^= 1 << (turing.tpos % 8);
        //shift_data(turing.tape, 3);
        
        disp_7seg_digit(turing.state >> 1);
        turing.disp_tape();

        /*
        if(btn_read() & BTN_CTR){
            //TAPE_MV_UP;
            turing.turing_step();
        }
        if(btn_read() & BTN_DL){
            turing.tape_move_dn();
        }
        if(btn_read() & BTN_DR){
            turing.tape_move_up();
        }
        if(btn_read() & BTN_DU){
            turing.tape_flipsym();
        }
        if(btn_read() & BTN_SR){
            turing.state = 0;
        }
        */
        
        delay_ms(20);        
    }
}
