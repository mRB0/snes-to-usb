#include "joypad.h"

#include <stdio.h>
#include <stdint.h>

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>

Joypad joypad;

static uint16_t const Joypad_clk_len = 0x2;
static uint16_t const Joypad_read_delay = 0x8;

static inline void enable_timer3_interrupt() {
    TIMSK3 |= _BV(OCIE3A);
}

static inline void disable_timer3_interrupt() {
    TIMSK3 &= ~_BV(OCIE3A);
}

// Reset timer3 with a new compare interrupt value
static void reset_timer3(uint16_t compare) {
    disable_timer3_interrupt();
    OCR3A = compare;
    
    // Reset the counter
    TCNT3 = 0;

    // Clear the interrupt in case it became set while disabled
    TIFR3 |= _BV(OCF3A);

    enable_timer3_interrupt();
}

// Set up timer3 for joypad interfacing
static void setup_timer3() {
    disable_timer3_interrupt();
    TCCR3A = 0x00;
    // WGM3 = 0100 (clear timer on compare match)
    TCCR3A &= ~_BV(WGM30);
    TCCR3A &= ~_BV(WGM31);
    TCCR3B |= _BV(WGM32);
    TCCR3B &= ~_BV(WGM33);

    // CS3 = 101 = clkIO/1024 (highest prescale)
    TCCR3B |= _BV(CS32);
    TCCR3B &= ~_BV(CS31);
    TCCR3B |= _BV(CS30);
    
    OCR3A = 0xffff;
}

//
// Joypad class implementation
//

Joypad::Joypad() : input_ready(false), input_value_P1(0), input_value_P2(0), input_presses_P1(0), input_presses_P2(0) { }

void Joypad::init() {
    // PORTB[3,2] = P1, P2 CLK out (normal high, strobe low)
    // PORTB1 = LAT out (normal low, strobe high)
    // PORTB[4,5] = P1, P2 DATA in (actuated button = low)
    
    DDRB |= _BV(DDB2) | _BV(DDB3);
    DDRB |= _BV(DDB1);
    DDRB &= ~(_BV(DDB4) | _BV(DDB5));

    PORTB |= _BV(PB2) | _BV(PB3);
    PORTB &= ~_BV(PB1);
    PORTB |= _BV(PB4) | _BV(PB5); // activate pull-ups (so absent controllers read with no buttons pressed)

    setup_timer3();
}

KeyState const Joypad::get_pressed_P1() {
    uint16_t presses = input_presses_P1;
    input_presses_P1 = 0;
    
    return KeyState(presses);
}

KeyState const Joypad::get_pressed_P2() {
    uint16_t presses = input_presses_P2;
    input_presses_P2 = 0;
    
    return KeyState(presses);
}

KeyState const Joypad::get_held_P2() {
    return KeyState(input_value_P1);
}

KeyState const Joypad::get_held_P1() {
    return KeyState(input_value_P2);
}

void Joypad::start_listening() {
    reset_timer3(Joypad_read_delay);
}

void Joypad::stop_listening() {
    disable_timer3_interrupt();
}

ISR(TIMER3_COMPA_vect) {
    // We track the next operation using this 'step' variable.  We
    // need to strobe the latch line, then toggle the clock while
    // reading data from the joypad, and then wait some time and
    // repeat.
    static uint8_t step = 0;
    
    // Stores the key state while we shift it in from the joypad.  After reading the last bit, we drop
    static uint16_t keystateP1 = 0;
    static uint16_t laststateP1 = 0;
    static uint16_t keystateP2 = 0;
    static uint16_t laststateP2 = 0;
    
    if (step == 0) {
        // begin read with latch strobe
        joypad.lat(true);
        keystateP1 = 0;
        keystateP2 = 0;
    } else if (step == 1) {
        // latch unstrobe
        joypad.lat(false);
    } else if (step >= 2 && step <= 33) {
        // read 16 bits
        if (step % 2 == 0) {
            // strobe clock
            joypad.clk(true);
        } else {
            // unstrobe clock and read
            keystateP1 |= joypad.readP1() << ((step - 3) / 2);
            keystateP2 |= joypad.readP2() << ((step - 3) / 2);
            joypad.clk(false);
            
            if (step == 33) {
                // last bit read; update global state
                joypad.input_value_P1 = keystateP1;
                joypad.input_value_P2 = keystateP2;
                joypad.input_ready = true;
                
                uint16_t new_keypresses_P1 = keystateP1 & ~laststateP1;
                laststateP1 = keystateP1;
                joypad.input_presses_P1 |= new_keypresses_P1;
                
                uint16_t new_keypresses_P2 = keystateP2 & ~laststateP2;
                laststateP2 = keystateP2;
                joypad.input_presses_P2 |= new_keypresses_P2;
            }
        }
    }
    
    step = (step + 1) % 34;
    if (step == 0) {
        // end of read; wait some time before next read
        reset_timer3(Joypad_read_delay);
    } else if (step == 1) {
        // beginning of read; increase clock until end
        reset_timer3(Joypad_clk_len);
    }
};
