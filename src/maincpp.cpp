#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <stdint.h>

#include <util/delay.h>

extern "C" {
#include "usb_gamepad.h"
}

#include "joypad.h"

#include "maincpp.h"

#ifndef NULL
#define NULL ((void *)0)
#endif



#define CPU_PRESCALE(n)	(CLKPR = 0x80, CLKPR = (n))
#define LED_CONFIG	(DDRD |= (1<<6))
#define LED_OFF		(PORTD &= ~(1<<6))
#define LED_ON		(PORTD |= (1<<6))


static void setup(void) {

    LED_CONFIG;
    LED_ON;
    
    // Clear the watchdog
    MCUSR &= ~_BV(WDRF);
    wdt_disable();
    wdt_reset();
    
    // 16 MHz clock speed
	CPU_PRESCALE(0);

    // Configure PORTB[0:6] as inputs.
    DDRB = 0x00;
    // Turn on internal pull-ups on PORTB[0:6].  This means we will read
    // them as logic high when the switches are open.  Logic low means
    // the switch is pressed (ie. active low).
    PORTB = 0x7f;

    LED_OFF;
    
	// Initialize USB, and then wait for the host to set
	// configuration.  If the Teensy is powered without a PC connected
	// to the USB port, this will wait forever.
	usb_init();
	while (!usb_configured()) /* wait */ ;

    LED_ON;
    
	// Wait an extra second for the PC's operating system to load
	// drivers and do whatever it does to actually be ready for input.
	// Show a little light show during this time.
    for(uint8_t i = 0; i < 5; i++) {
        _delay_ms(180);
        LED_OFF;
        _delay_ms(20);
        LED_ON;
    }
    
    cli();

    joypad.init();
    
    /* // Configure timer 0 to give us ticks */
	/* TCCR0A = 0x00; */
	/* TCCR0B = Timer0Overflow & 0x07; */
	/* TIMSK0 = (1<<TOIE0); // use the overflow interrupt only */
    /* _timer0_fired = 0; */
}



static void run(void) {
    wdt_reset();
    //wdt_enable(WDTO_1S);

    sei();

    joypad.start_listening();

    uint16_t laststate_P1 = 0xffff, laststate_P2 = 0xffff;
    
    for(;;) {
        while (!joypad.input_ready) {
        }

        joypad.input_ready = false;

        for (int i = 0; i < 2; i++) {
            int8_t (*send_fn)();
            struct gamepad *gamepad;
            uint16_t *laststate;
            KeyState nowstate(0);
            
            if (i == 0) {
                send_fn = usb_gamepad1_send;
                gamepad = &gamepad1;
                laststate = &laststate_P1;
                nowstate = joypad.get_held_P1();
            } else {
                send_fn = usb_gamepad2_send;
                gamepad = &gamepad2;
                laststate = &laststate_P2;
                nowstate = joypad.get_held_P2();
            }

            if (*laststate != nowstate._keyvalue) {
                // only send keys if they've changed
                *laststate = nowstate._keyvalue;

                if (nowstate.key_up()) {
                    gamepad->y_axis = -127;
                } else if (nowstate.key_down()) {
                    gamepad->y_axis = 127;
                } else {
                    gamepad->y_axis = 0;
                }

                if (nowstate.key_left()) {
                    gamepad->x_axis = -127;
                } else if (nowstate.key_right()) {
                    gamepad->x_axis = 127;
                } else {
                    gamepad->x_axis = 0;
                }

                uint8_t buttons = 0;
                
                if (nowstate.key_a()) {
                    buttons |= (0x01 << 0);
                }
                
                if (nowstate.key_b()) {
                    buttons |= (0x01 << 1);
                }
                
                if (nowstate.key_x()) {
                    buttons |= (0x01 << 2);
                }
                
                if (nowstate.key_y()) {
                    buttons |= (0x01 << 3);
                }
                    
                if (nowstate.key_l()) {
                    buttons |= (0x01 << 4);
                }
                
                if (nowstate.key_r()) {
                    buttons |= (0x01 << 5);
                }
                
                if (nowstate.key_start()) {
                    buttons |= (0x01 << 6);
                }
                
                if (nowstate.key_select()) {
                    buttons |= (0x01 << 7);
                }

                gamepad->buttons = buttons;
                
                send_fn();
            }
                
        }
        
    }
    
    for(;;) {
        for(int i = 0; i < 2; i++) {
            int8_t (*send_fn)();
            struct gamepad *active_gamepad;
            
            if (i == 0) {
                send_fn = usb_gamepad1_send;
                active_gamepad = &gamepad1;
            } else {
                send_fn = usb_gamepad2_send;
                active_gamepad = &gamepad2;
            }
        
            active_gamepad->x_axis = -127;
            send_fn();
            _delay_ms(250);
        
            active_gamepad->y_axis = -127;
            send_fn();
            _delay_ms(250);
        
            active_gamepad->x_axis = 127;
            send_fn();
            _delay_ms(250);
        
            active_gamepad->y_axis = 127;
            send_fn();
            _delay_ms(250);
        
            active_gamepad->x_axis = 0;
            send_fn();
            _delay_ms(250);
        
            active_gamepad->y_axis = 0;
            send_fn();
            _delay_ms(250);
        
            active_gamepad->buttons = 0x01;
            send_fn();
            _delay_ms(250);
        
            active_gamepad->buttons = 0x02;
            send_fn();
            _delay_ms(250);
        
            active_gamepad->buttons = 0x04;
            send_fn();
            _delay_ms(250);
        
            active_gamepad->buttons = 0x08;
            send_fn();
            _delay_ms(250);
        
            active_gamepad->buttons = 0x10;
            send_fn();
            _delay_ms(250);
        
            LED_OFF;
            active_gamepad->buttons = 0x20;
            send_fn();
            _delay_ms(250);
        
            LED_ON;
            active_gamepad->buttons = 0x40;
            send_fn();
            _delay_ms(250);
        
            LED_OFF;
            active_gamepad->buttons = 0x80;
            send_fn();
            _delay_ms(250);
        
            LED_ON;
            active_gamepad->buttons = 0xff;
            send_fn();
            _delay_ms(250);
        
            LED_OFF;
            active_gamepad->buttons = 0x55;
            send_fn();
            _delay_ms(250);
        
            LED_ON;
            active_gamepad->buttons = 0xAA;
            send_fn();
            _delay_ms(250);
        
            LED_ON;
            active_gamepad->buttons = 0x00;
            send_fn();
            _delay_ms(250);
        
        }
    }
}

int maincpp(void) {
    setup();
    run();
    return 0;
}


// Timer 0 overflow interrupt handler.
ISR(TIMER0_OVF_vect) {
}
