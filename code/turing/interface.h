#include <avr/io.h>
#include <avr/interrupt.h>

#define SET_DISP        PORTB &= ~0x01; DDRB |= 0x01
#define RST_DISP        PORTB |= 0x01;  DDRB &= ~0x01

#define SET_BTNR        PORTB &= ~0x02; DDRB |= 0x02
#define RST_BTNR        PORTB |= 0x02;  DDRB &= ~0x02

#define SEG_B_MASK      0x00 //0b00000000
#define SEG_C_MASK      0x3e //0b00111110
#define SEG_D_MASK      0xc0 //0b11000000

#define SEG_DISP_T      1

/*
 * -bar--->--avr
 * Vcc  -->  Vcc
 * STR  -->  D2
 * D    -->  D3
 * CLK  -->  D4
 * GND  -->  GND (D5)
 *
*/

#define SHIFT_PORT     PORTD
#define SHIFT_STR      (1<<2)
#define SHIFT_D        (1<<3)
#define SHIFT_CLK      (1<<4)

#define SHIFT_CLK_SET  SHIFT_PORT |= SHIFT_CLK
#define SHIFT_CLK_RST  SHIFT_PORT &= ~SHIFT_CLK

#define SHIFT_D_SET    SHIFT_PORT |= SHIFT_D
#define SHIFT_D_RST    SHIFT_PORT &= ~SHIFT_D

#define SHIFT_STR_SET  SHIFT_PORT |= SHIFT_STR
#define SHIFT_STR_RST  SHIFT_PORT &= ~SHIFT_STR

#define SHIFT_STR_EN   SHIFT_STR_SET;SHIFT_STR_RST

#define POT_PIN        0

void interface_setup(void);
void disp_7seg(uint8_t);
int read_pot(void);

void _shift_byte(uint8_t);
void shift_byte(uint8_t);
void shift_data(uint8_t *v, uint8_t);



#define BTN_READ            0x01
#define BTN_DEBOUNCE_TIME   30      // in ms

uint8_t btnPress;
uint8_t btnState;
uint8_t btnDebounce;
unsigned long btnDebounceTimeout;

void btn_poll(void);
uint8_t btn_read(void);


void delay_ms_poll(int);

/* Shamelessly stolen from the Arduino Wiring library */
unsigned long millis(void);
void delay_ms(unsigned long);
void init_timers(void);
