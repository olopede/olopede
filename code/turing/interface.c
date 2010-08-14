#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "interface.h"




void interfaceSetup(){
    DDRD = 0xff;
    DDRC = 0xfe;
    DDRB = 0xff;
}

void disp7Seg(uint8_t v){
    //C[1:5], D[6:7]
    
    #if SEG_B_MASK != 0
    PORTB &= ~SEG_B_MASK;
    PORTB |= SEG_B_MASK & v;
    #endif
    
    #if SEG_C_MASK != 0
    PORTC &= ~SEG_C_MASK;
    PORTC |= SEG_C_MASK & v;
    #endif
    
    #if SEG_D_MASK != 0
    PORTD &= ~SEG_D_MASK;
    PORTD |= SEG_D_MASK & v;
    #endif
}

void shiftByte(uint8_t v){
    uint8_t i;
    for(i = 0; i < 8; i++){
        SHIFT_CLK_RST;
        if(v & (1<<i))
            SHIFT_D_SET;
        else
            SHIFT_D_RST;
        SHIFT_CLK_SET;
    }
    SHIFT_STR_SET;
    SHIFT_STR_RST;
}

int readPot(void)
{
    DDRC &= ~(1 << POT_PIN);
	uint8_t low, high;
    ADCSRA |= (1 << ADEN);
	// set the analog reference (high two bits of ADMUX) and select the
	// channel (low 4 bits).  this also sets ADLAR (left-adjust result)
	// to 0 (the default).
	ADMUX = (1 << 6) | POT_PIN;

	// without a delay, we seem to read from the wrong channel
	//delay(1);

	// start the conversion
	ADCSRA |= 1 << ADSC;

	// ADSC is cleared when the conversion finishes
	while (ADCSRA & (1 << ADSC));

	// we have to read ADCL first; doing so locks both ADCL
	// and ADCH until ADCH is read.  reading ADCL second would
	// cause the results of each conversion to be discarded,
	// as ADCL and ADCH would be locked when it completed.
	low = ADCL;
	high = ADCH;

	// combine the two bytes
	return (high << 8) | low;
}

