#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "interface.h"

int main(void){
    uint8_t i;
    
    interfaceSetup();
    
    SET_DISP;
    for(i = 0;;i++){
        
        disp7Seg(readPot() & 0xff);
        shiftByte(i);
        _delay_ms(50);
        
    }
    return 0;
}
