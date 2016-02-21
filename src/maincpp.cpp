#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <stdint.h>

#include <util/delay.h>

#include "iusb_controller.h"
#include "usb_controller_as_gamepad.h"

#include "joypad.h"

#include "maincpp.h"

#ifndef NULL
#define NULL ((void *)0)
#endif



#define CPU_PRESCALE(n)	(CLKPR = 0x80, CLKPR = (n))
#define LED_CONFIG	(DDRD |= (1<<6))
#define LED_OFF		(PORTD &= ~(1<<6))
#define LED_ON		(PORTD |= (1<<6))

enum DeviceMode {
    DeviceModeGamepad,
    DeviceModeKeyboard
};

static DeviceMode _device_mode;

static IUSBController *volatile _usb_controller; // reference kept so we can dispatch to ISRs

static void run(IUSBController &ctrl) {
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
            int8_t (IUSBController::*send_fn)();
            struct gamepad *gamepad;
            uint16_t *laststate;
            KeyState nowstate(0);
            
            if (i == 0) {
                send_fn = &IUSBController::gamepad1_send;
                gamepad = &ctrl.gamepad1;
                laststate = &laststate_P1;
                nowstate = joypad.get_held_P1();
            } else {
                send_fn = &IUSBController::gamepad2_send;
                gamepad = &ctrl.gamepad2;
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
                
                (ctrl.*send_fn)();
            }
                
        }
        
    }
}


static void setup_phase2(IUSBController &ctrl) {
	// Initialize USB, and then wait for the host to set
	// configuration.  If the Teensy is powered without a PC connected
	// to the USB port, this will wait forever.
	ctrl.init();
	while (!ctrl.is_configured()) /* wait */ ;

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

    run(ctrl);
}

static void setup_phase1(void) {

    LED_CONFIG;
    LED_ON;
    
    // Clear the watchdog
    MCUSR &= ~_BV(WDRF);
    wdt_disable();
    wdt_reset();
    
    // 16 MHz clock speed
	CPU_PRESCALE(0);

    // Find out whether we're going to be a keyboard or a game controller.
    DDRF &= ~(_BV(DDF2) | _BV(DDF6)); // in: PORTF[2,6]
    DDRF |= _BV(DDF4); // out (low): PORTF4
    
    PORTF |= _BV(PF2) | _BV(PF6); // pull-ups: PORTF[2,6]
    PORTF &= ~_BV(PF4); // out low: PORTF4

    // PORTF2: high = gamepad
    // All that, and we only read PORTF2? hah!
    _device_mode = (PINF & _BV(PINF2)) ? DeviceModeGamepad : DeviceModeKeyboard;

    // Done with PORTF; turn it off.
    DDRF &= ~_BV(DDF4);
    PORTF &= ~(_BV(PF2) | _BV(PF4) | _BV(PF6));

    LED_OFF;

    if (_device_mode == DeviceModeGamepad) {
        USBControllerAsGamepad controller;
        _usb_controller = &controller;
        setup_phase2(controller);
        _usb_controller = (IUSBController *volatile)0;
    }
    
    
}

int maincpp(void) {
    setup_phase1();
    return 0;
}


// Timer 0 overflow interrupt handler.
ISR(TIMER0_OVF_vect) {
}

ISR(USB_GEN_vect) {
    _usb_controller->ISR_USB_GEN_vect();
}

ISR(USB_COM_vect) {
    _usb_controller->ISR_USB_COM_vect();
}
