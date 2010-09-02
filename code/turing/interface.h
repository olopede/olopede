#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#ifndef interface_h_include
#define interface_h_include

#define SET_DISP1       PORTB &= ~0x01; DDRB |= 0x01
#define RST_DISP1       PORTB |= 0x01;  DDRB &= ~0x01

#define SET_BTNR        PORTB &= ~0x02; DDRB |= 0x02
#define RST_BTNR        PORTB |= 0x02;  DDRB &= ~0x02

#define SEG_B_MASK      0x00 //0b00000000
#define SEG_C_MASK      0x3e //0b00111110
#define SEG_D_MASK      0xc0 //0b11000000

#define SEG_DISP_T      0

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
void disp_7seg_digit(uint8_t);

void _shift_byte(uint8_t);
void shift_byte(uint8_t);
void shift_data(uint8_t *v, uint8_t);

extern uint8_t asdf;

#define BTN_TIMER_CMP       250

#define BTN_READ            0x01
#define BTN_DEBOUNCE_TIME   30      // in ms

#define BTN_CTR             0x04
#define BTN_SL              0x08
#define BTN_SR              0x02
#define BTN_DU              0x10
#define BTN_DR              0x20
#define BTN_DD              0x40
#define BTN_DL              0x80


void btn_poll(void);
uint8_t btn_read(void);

/* Button event functions */

// Default (nothing)
void do_nothing(void);

// Presses
extern void (*btn_ctr_press)(void);
extern void (*btn_sl_press)(void);
extern void (*btn_sr_press)(void);
extern void (*btn_dl_press)(void);
extern void (*btn_dr_press)(void);

extern void (*btn_ctr_hold)(void);
extern void (*btn_sl_hold)(void);
extern void (*btn_sr_hold)(void);
extern void (*btn_dl_hold)(void);
extern void (*btn_dr_hold)(void);



int pot_read(void);

/* Shamelessly stolen from the Arduino Wiring library */
unsigned long millis(void);
void delay_ms(unsigned long);
void init_timers(void);

#endif //interface_h_include
