#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "interface.h"

#define NOP asm("nop")                   

uint8_t asdf;
uint8_t btnPress;
uint8_t btnState;
//uint8_t btnDebounce;
//unsigned long btnDebounceTimeout;

static uint8_t seg_digits[11] = {0x7E, 0x0C, 0xB6, 0x9E, 0xCC, 0xDA, 0xFA, 0x0E, 0xFE, 0xCE, 0xEC}; //PROGMEM?

void (*btn_ctr_press)(void) = &do_nothing;
void (*btn_sl_press)(void) = &do_nothing;
void (*btn_sr_press)(void) = &do_nothing;
void (*btn_dl_press)(void) = &do_nothing;
void (*btn_dr_press)(void) = &do_nothing;

void (*btn_ctr_hold)(void) = &do_nothing;
void (*btn_sl_hold)(void) = &do_nothing;
void (*btn_sr_hold)(void) = &do_nothing;
void (*btn_dl_hold)(void) = &do_nothing;
void (*btn_dr_hold)(void) = &do_nothing;

void interface_setup(){

    init_timers();
    DDRD = 0x1C;
    OCR1A = BTN_TIMER_CMP;
    
    //DDRC = 0xfe;
    //DDRB = 0xff;
    
    // For analog read
    DDRC &= ~(1 << POT_PIN);
    ADCSRA |= (1 << ADEN);
	ADMUX = (1 << 6) | POT_PIN;
}

ISR(TIMER1_COMPA_vect){
    // Timer1 overflow -- poll buttons
    // Disable interrupts while polling
    // Some button events may take a while

    cli();
    btn_poll();
    asdf++;
    sei();
}

void disp_7seg(uint8_t v){
    // Display a figure, using...
    /*    0bgfedcbaP
            --a--
           |     |
           f     b
           |--g--|
           e     c
           |     |
            --d--  . P
    */
    //C[1:5], D[6:7]
    
    // If buttons are enabled, turn them off & enable display
    RST_BTNR;
    SET_DISP1;
    #if SEG_B_MASK != 0
    DDRB |= SEG_B_MASK;
    PORTB &= ~SEG_B_MASK;
    PORTB |= SEG_B_MASK & v;
    #endif
    
    #if SEG_C_MASK != 0
    DDRC |= SEG_C_MASK;
    PORTC &= ~SEG_C_MASK;
    PORTC |= SEG_C_MASK & v;
    #endif
    
    #if SEG_D_MASK != 0
    DDRD |= SEG_D_MASK;
    PORTD &= ~SEG_D_MASK;
    PORTD |= SEG_D_MASK & v;
    #endif
    
    #if SEG_DISP_T > 0
    delay_ms_poll(SEG_DISP_T);
    #endif
}

void disp_7seg_digit(uint8_t v){
    if(v == 0x1F)
        v = 10;
    if(v > 10)
        v = 8;
    
    disp_7seg(seg_digits[v]);
}


void _shift_byte(uint8_t v){
    uint8_t i;
    for(i = 0x01; i; i <<= 1){  //TODO: Should probably be reversed in next revision
        if(v & i)
            SHIFT_D_SET;
        else
            SHIFT_D_RST;
        
        SHIFT_CLK_SET;
        SHIFT_CLK_RST;
    }
}
void shift_byte(uint8_t v){
    _shift_byte(v);
    SHIFT_STR_EN;
}

void shift_data(uint8_t *v, uint8_t len){
    for(uint8_t i = 0; i < len; i++){
        _shift_byte(v[i]);
    }
    SHIFT_STR_EN;
}

void btn_poll(){
    // Poll the button states
    // Called from interrupt
    
    
    // Disable 7 seg display in order to read btn states
    RST_DISP1;
    SET_BTNR;
    
    uint8_t _btnState = btnState;
    btnState = 0x00;

    if(btnPress & BTN_READ){
        btnPress = 0x00;
    }

    #if SEG_B_MASK != 0
    uint8_t _portb = PORTB;         // Previous state (leave things as they are found)
    DDRB &= ~SEG_B_MASK;            // Set to input
    PORTB |= SEG_B_MASK;            // Disable pullups
    #endif
    
    #if SEG_C_MASK != 0
    uint8_t _portc = PORTC;
    DDRC &= ~SEG_C_MASK;
    PORTC |= SEG_C_MASK;
    #endif
    
    #if SEG_D_MASK != 0
    uint8_t _portd = PORTD;
    DDRD &= ~SEG_D_MASK;
    PORTD |= SEG_D_MASK;
    #endif
    
    // Experiments show that 7 nops are nessassary to allow pullups to take 
    // effect at 16Mhz; 10 for good measure, otherwise it'd really screw up
    NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;   // 10 NOPs
    
    #if SEG_B_MASK != 0
    btnState |= (~PINB & SEG_B_MASK);    // Read. Low bits are "active". 
    PORTB = _portb;                      // Reset to previous state
    DDRB |= SEG_B_MASK;
    #endif
    
    #if SEG_C_MASK != 0
    btnState |= (~PINC & SEG_C_MASK);
    PORTC = _portc;
    DDRC |= SEG_C_MASK;
    #endif
    
    #if SEG_D_MASK != 0
    btnState |= (~PIND & SEG_D_MASK);
    PORTD = _portd;
    DDRD |= SEG_D_MASK;
    #endif
    
    // Enable 7 seg display output
    RST_BTNR;       
    SET_DISP1;
    
    //btnPress |= (_btnState ^ btnState) & btnState; // Only take rising edge
    btnPress |= ~_btnState & btnState;               // Logically equivalent
    
    // Call event functions
    if(btnState & BTN_CTR){
        if(_btnState & BTN_CTR)
            btn_ctr_hold();
        else
            btn_ctr_press();
    }
    if(btnState & BTN_SL){
        if(_btnState & BTN_SL)
            btn_sl_hold();
        else
            btn_sl_press();
    }
    if(btnState & BTN_SR){
        if(_btnState & BTN_SR)
            btn_sr_hold();
        else
            btn_sr_hold();
    }
    if(btnState & BTN_DR){
        if(_btnState & BTN_DR)
            btn_dr_hold();
        else
            btn_dl_press();
    }
    if(btnState & BTN_DL){
        if(_btnState & BTN_DL)
            btn_dl_hold();
        else
            btn_dl_press();
    }
    
    
    
    
}

uint8_t btn_read(){
    // btnPress should have BTN_READ bit unset
    btnPress |= BTN_READ;
    //btnRead = btnPress;
    return btnPress;
}

void do_nothing(){
    // So exciting!
    // Default button action
}


int pot_read(void){
    uint8_t low;

    /* -- Moved to init; only called once
    DDRC &= ~(1 << POT_PIN);
	
    ADCSRA |= (1 << ADEN);
	// set the analog reference (high two bits of ADMUX) and select the
	// channel (low 4 bits).  this also sets ADLAR (left-adjust result)
	// to 0 (the default).
	ADMUX = (1 << 6) | POT_PIN;
    */

	// without a delay, we seem to read from the wrong channel
	//delay(1);

	// start the conversion
	ADCSRA |= 1 << ADSC;

	// ADSC is cleared when the conversion finishes
	while (ADCSRA & (1 << ADSC)) ;

	// we have to read ADCL first; doing so locks both ADCL
	// and ADCH until ADCH is read.  reading ADCL second would
	// cause the results of each conversion to be discarded,
	// as ADCL and ADCH would be locked when it completed.
	low = ADCL;
	//high = ADCH;

	// combine the two bytes
	return (ADCH << 8) | low;
}


/* Shamelessly stolen from the Arduino Wiring library */
// the prescaler is set so that timer0 ticks every 64 clock cycles, and the
// the overflow handler is called every 256 ticks.
#define MICROSECONDS_PER_TIMER0_OVERFLOW (1024)
#define MILLIS_INC (MICROSECONDS_PER_TIMER0_OVERFLOW / 1000)

// the fractional number of milliseconds per timer0 overflow. we shift right
// by three to fit these numbers into a byte. (for the clock speeds we care
// about - 8 and 16 MHz - this doesn't lose precision.)
#define FRACT_INC ((MICROSECONDS_PER_TIMER0_OVERFLOW % 1000) >> 3)
#define FRACT_MAX (1000 >> 3)

volatile unsigned long timer0_overflow_count = 0;
volatile unsigned long timer0_millis = 0;
static unsigned char timer0_fract = 0;

SIGNAL(TIMER0_OVF_vect)
{
	// copy these to local variables so they can be stored in registers
	// (volatile variables must be read from memory on every access)
	unsigned long m = timer0_millis;
	unsigned char f = timer0_fract;

	m += MILLIS_INC;
	f += FRACT_INC;
	if (f >= FRACT_MAX) {
		f -= FRACT_MAX;
		m += 1;
	}

	timer0_fract = f;
	timer0_millis = m;
	timer0_overflow_count++;
}
unsigned long millis(){
	unsigned long m;
	uint8_t oldSREG = SREG;

	// disable interrupts while we read timer0_millis or we might get an
	// inconsistent value (e.g. in the middle of a write to timer0_millis)
	cli();
	m = timer0_millis;
	SREG = oldSREG;

	return m;
}

void delay_ms(unsigned long ms){
    /* this function is cheaper than _delay_ms for flash */
	unsigned long start = millis();
	
	while (millis() - start <= ms) ;
}

void init_timers(){
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif
	// this needs to be called before setup() or some functions won't
	// work there
	sei();
	
	// on the ATmega168, timer 0 is also used for fast hardware pwm
	// (using phase-correct PWM would mean that timer 0 overflowed half as often
	// resulting in different millis() behavior on the ATmega8 and ATmega168)
#if !defined(__AVR_ATmega8__)
	sbi(TCCR0A, WGM01);
	sbi(TCCR0A, WGM00);
#endif  
	// set timer 0 prescale factor to 64
#if defined(__AVR_ATmega8__)
	sbi(TCCR0, CS01);
	sbi(TCCR0, CS00);
#else
	sbi(TCCR0B, CS01);
	sbi(TCCR0B, CS00);
#endif
	// enable timer 0 overflow interrupt
#if defined(__AVR_ATmega8__)
	sbi(TIMSK, TOIE0);
#else
	sbi(TIMSK0, TOIE0);
#endif

	// timers 1 and 2

#if F_CPU == 8000000
	// set timer 1 prescale factor to 1024
	sbi(TCCR1B, CS12);
	cbi(TCCR1B, CS11);
	sbi(TCCR1B, CS10);
#elif F_CPU == 1000000 || F_CPU == 2000000
    //set timer 1 prescale factor to 256
    sbi(TCCR1B, CS12);
	cbi(TCCR1B, CS11);
	cbi(TCCR1B, CS10);  
#else
    #warning "F_CPU value not supported. Buttons may not work"
    // set timer 1 prescale factor to 64
    cbi(TCCR1B, CS12);
	sbi(TCCR1B, CS11);
	sbi(TCCR1B, CS10); 
#endif
	sbi(TCCR1B, WGM12);  // Wave generation mode 4 -- CTC
	cbi(TCCR1B, WGM13);
	cbi(TCCR1A, WGM11);
    cbi(TCCR1A, WGM10);
    
    sbi(TIMSK1, OCIE1A);
    sbi(TCCR1A, COM1A1);
    //TCCR1B = (1<<WGM13) | (1<<WGM12); 

	// put timer 1 in 8-bit phase correct pwm mode
	//sbi(TCCR1A, WGM10);

	// set timer 2 prescale factor to 64
#if defined(__AVR_ATmega8__)
	sbi(TCCR2, CS22);
#else
	sbi(TCCR2B, CS22);
#endif
	// configure timer 2 for phase correct pwm (8-bit)
#if defined(__AVR_ATmega8__)
	sbi(TCCR2, WGM20);
#else
	sbi(TCCR2A, WGM20);
#endif
    

	// set a2d prescale factor to 128
	// 16 MHz / 128 = 125 KHz, inside the desired 50-200 KHz range.
	// XXX: this will not work properly for other clock speeds, and
	// this code should use F_CPU to determine the prescale factor.
	// prescale factor = 2 ^ (SUM of ADPS values)
	// 1 1 1 ==> 2 ^ 7 = 128
	// 0 1 1 ==> 2 ^ 3 = 8
#if F_CPU == 16000000
	sbi(ADCSRA, ADPS2); // 4
	sbi(ADCSRA, ADPS1); // 2
	sbi(ADCSRA, ADPS0); // 1
#elif F_CPU == 8000000
    sbi(ADCSRA, ADPS2);
	cbi(ADCSRA, ADPS1);
	cbi(ADCSRA, ADPS0);
#elif F_CPU == 1000000
 	cbi(ADCSRA, ADPS2);
	sbi(ADCSRA, ADPS1);
	sbi(ADCSRA, ADPS0);
#else
    #warning "F_CPU value not supported. Analog readings are not accurate"
    sbi(ADCSRA, ADPS2); // default to 8Mhz
	cbi(ADCSRA, ADPS1);
	cbi(ADCSRA, ADPS0);
#endif   
	// enable a2d conversions
	sbi(ADCSRA, ADEN);

	// the bootloader connects pins 0 and 1 to the USART; disconnect them
	// here so they can be used as normal digital i/o; they will be
	// reconnected in Serial.begin()
#if defined(__AVR_ATmega8__)
	UCSRB = 0;
#else
	UCSR0B = 0;
#endif

    
}
