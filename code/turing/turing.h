#include <avr/io.h>
#include <avr/interrupt.h>

#include "interface.h"

#define LIGHTBARS     3            // How many lightbars of 8 are visible
#define TAPESIZE      64           // Total size of tape, not always visible.
#define BTAPESIZE     (TAPESIZE/8)  // How large is the array to hold the tapesize

uint8_t tape[BTAPESIZE];
//uint16_t tpos;


void tape_clear(void);
void tape_putint(uint16_t);
void disp_tape(void);
