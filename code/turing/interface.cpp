#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "interface.h"

#define NOP asm("nop")                   


uint8_t btnPress;
uint8_t btnState;
//uint8_t btnDebounce;
//unsigned long btnDebounceTimeout;
static uint8_t seg_digits[11] = {0x7E, 0x0C, 0xB6, 0x00, 0x00, 0x00, 0x00, 0xFE, 0xFE, 0x00, 0xEC};


void interface_setup(){
    init_timers();
    DDRD = 0x1C;
    //DDRC = 0xfe;
    //DDRB = 0xff;
}

SIGNAL(TIMER1_OVF_vect){
    // Timer1 overflow -- poll buttons
    btn_poll();
}

void disp_7seg(uint8_t v){
    //C[1:5], D[6:7]
    RST_BTNR;
    SET_DISP;
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
    
    #if SEG_DISP_T != 0
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
int read_pot(void){
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
	while (ADCSRA & (1 << ADSC)) ;

	// we have to read ADCL first; doing so locks both ADCL
	// and ADCH until ADCH is read.  reading ADCL second would
	// cause the results of each conversion to be discarded,
	// as ADCL and ADCH would be locked when it completed.
	low = ADCL;
	high = ADCH;

	// combine the two bytes
	return (high << 8) | low;
}

void btn_poll(){
    uint8_t _portb, _portc, _portd, _btnState; // Previous states

    _portb = PORTB;
    _portc = PORTC;
    _portd = PORTD;
    
    
    RST_DISP;
    SET_BTNR;
    
    _btnState = btnState;
    btnState = 0x00;

    if(btnPress & BTN_READ){
        btnPress = 0x00;
    }

    #if SEG_B_MASK != 0
    DDRB &= ~SEG_B_MASK;             // Set to input
    PORTB |= SEG_B_MASK;            // Disable pullups
    #endif
    
    #if SEG_C_MASK != 0
    DDRC &= ~SEG_C_MASK;
    PORTC |= SEG_C_MASK;
    #endif
    
    #if SEG_D_MASK != 0
    DDRD &= ~SEG_D_MASK;
    PORTD |= SEG_D_MASK;
    #endif
    
    
    // Experiments show that 7 nops are nessassary to allow pullups to take effect at 16Mhz
    // 10 for good measure, otherwise it'd suck.
    NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;   // 10 NOPs
    
    
    btnState |= (~PINB & SEG_B_MASK); // Read. Low bits are "active". 
    btnState |= (~PINC & SEG_C_MASK);
    btnState |= (~PIND & SEG_D_MASK);
    btnPress |= (_btnState ^ btnState) & btnState; // Rising edge
    
    
    // Reset everything to the way it was & output for 7 seg
    PORTB = _portb;
    PORTC = _portc;
    PORTD = _portd;
    
    DDRB |= SEG_B_MASK;
    DDRC |= SEG_C_MASK;
    DDRD |= SEG_D_MASK;

    RST_BTNR;
    SET_DISP;
}

uint8_t btn_read_cached(){
    // btnPress should have BTN_READ bit unset
    btnPress |= BTN_READ;
    //btnRead = btnPress;
    #if BTN_READ == 0x01
    return btnPress >> 1;
    #else
    #warning "BTN_READ bit is not yet supported. expect strange behavior from btnPress!"
    return btnPress;
    #endif
}

uint8_t btn_read(){
    btn_poll();
    return btn_read_cached();
}



/* Use delay_ms_poll instead of _delay_ms
 * It executes "background" functions periodically
 *
 * Although interrupts could be used, the btnPoll code could cause problems
 * with the analog input & 7seg code, because they use the same pins
 */
void delay_ms_poll(int t){
    btn_poll();
    for(;t > BTN_DEBOUNCE_TIME; t -= BTN_DEBOUNCE_TIME){ 
        delay_ms(BTN_DEBOUNCE_TIME);
        btn_poll();
    }
    delay_ms(t);
    btn_poll();
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

	// timers 1 and 2 are used for phase-correct hardware pwm
	// this is better for motors as it ensures an even waveform
	// note, however, that fast pwm mode can achieve a frequency of up
	// 8 MHz (with a 16 MHz clock) at 50% duty cycle

	// set timer 1 prescale factor to 64
	sbi(TCCR1B, CS11);
	sbi(TCCR1B, CS10);
	// put timer 1 in 8-bit phase correct pwm mode
	sbi(TCCR1A, WGM10);

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

#if defined(__AVR_ATmega1280__)
	// set timer 3, 4, 5 prescale factor to 64
	sbi(TCCR3B, CS31);	sbi(TCCR3B, CS30);
	sbi(TCCR4B, CS41);	sbi(TCCR4B, CS40);
	sbi(TCCR5B, CS51);	sbi(TCCR5B, CS50);
	// put timer 3, 4, 5 in 8-bit phase correct pwm mode
	sbi(TCCR3A, WGM30);
	sbi(TCCR4A, WGM40);
	sbi(TCCR5A, WGM50);
#endif

	// set a2d prescale factor to 128
	// 16 MHz / 128 = 125 KHz, inside the desired 50-200 KHz range.
	// XXX: this will not work properly for other clock speeds, and
	// this code should use F_CPU to determine the prescale factor.
	sbi(ADCSRA, ADPS2);
	sbi(ADCSRA, ADPS1);
	sbi(ADCSRA, ADPS0);

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
