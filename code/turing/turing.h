#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "interface.h"

#ifndef __turing_h_include__
#define __turing_h_include__

/*
 * Tape interface
 */

#define LIGHTBARS     3             // How many lightbars of 8 are visible
#define TAPESIZE      40             // Total size of tape, not always visible.
#define BTAPESIZE     (TAPESIZE/8)  // How large is the array to hold the tapesize
#define TPOS_OFFSET   LIGHTBARS*4   // How far from the edge to display the "current" center 

//#define TAPE_MV_DN    ++tpos; if(tpos >= TAPESIZE){ tpos = 0; }
//#define TAPE_MV_UP    if(tpos > 0){ tpos--; } else { tpos = TAPESIZE-1; }

class Turing {
  public:
    uint8_t tape[BTAPESIZE];
    uint16_t tpos;        // Position
    uint8_t state;        // Initial state
    uint8_t *states;      // State array
    
    Turing(void);
    
    void tape_clear(void);
    void tape_putchar(uint8_t);
    uint8_t tape_getsym(void);
    void tape_putsym(uint8_t);
    void tape_flipsym(void);
    void tape_move_up(void);
    void tape_move_dn(void);
    
    void disp_tape(void);
    
    void turing_step(void);
};

/*
void tape_clear(void);
void tape_putchar(uint8_t);
uint8_t tape_getsym(void);
void tape_putsym(uint8_t);
void tape_move_up(void);
void tape_move_dn(void);

void disp_tape(void);
*/
/*
 * Turing logic
 */


#define TURING_MAX_STATES   5       // How many states you want your turing machine to have
                                    // If greater than 32, then you cannot use the NOP instruction
                                    // Not strictly enforced, just for NOP
#define TURING_SYM_MASK 0x01
#define TURING_MV_MASK 0x80         // 0b1xxxxxxx for up, 0b0xxxxxxx for down          

#if TURING_MAX_STATES < 32              // Cannot use NOP if more than 31 states
#define TURING_NOP_MASK 0x40        // 0bx1xxxxxx for nop
#define TURING_STATE_MASK 0x3E

#else
#define TURING_STATE_MASK 0x7E

#endif

#define TURING_STATE_HALT TURING_STATE_MASK  // Which state is considered HALT?


// state - Q; sym - Y; direction - D
// states[state | sym] = direction | state | sym
// states[0bxxQQQQQY] = 0bDDQQQQQY   // With NOP, 32 states max
// states[0bxQQQQQQY] = 0bDQQQQQQY   // Without NOP, 64 states max
//uint8_t states[TURING_STATES << 1];  // Two items per state (sym 0, 1)
//uint8_t state;


#endif //__turing_h_include__
