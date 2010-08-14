#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define SET_DISP        PORTB &= ~0x01
#define RST_DISP        PORTB |= 0x01

#define SEG_B_MASK     0b00000000
#define SEG_C_MASK     0b00111110
#define SEG_D_MASK     0b11000000

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

#define POT_PIN        0

void interfaceSetup();
void disp7Seg(uint8_t v);
int readPo(void);
void shiftByte(uint8_t v);
