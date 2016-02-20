#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <stdint.h>

#define F_CPU 16000000UL // 16mhz
#include <util/delay.h>

#include "usb_gamepad.h"

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
    
	for(;;) {
        gamepad_xaxis = -127;
        usb_gamepad_send();
        _delay_ms(250);
        
        gamepad_yaxis = -127;
        usb_gamepad_send();
        _delay_ms(250);
        
        gamepad_xaxis = 127;
        usb_gamepad_send();
        _delay_ms(250);
        
        gamepad_yaxis = 127;
        usb_gamepad_send();
        _delay_ms(250);
        
        gamepad_xaxis = 0;
        usb_gamepad_send();
        _delay_ms(250);
        
        gamepad_yaxis = 0;
        usb_gamepad_send();
        _delay_ms(250);
        
        LED_ON;
        gamepad_buttons = 0x01;
        usb_gamepad_send();
        _delay_ms(250);
        
        LED_OFF;
        gamepad_buttons = 0x02;
        usb_gamepad_send();
        _delay_ms(250);
        
        LED_ON;
        gamepad_buttons = 0x04;
        usb_gamepad_send();
        _delay_ms(250);
        
        LED_OFF;
        gamepad_buttons = 0x08;
        usb_gamepad_send();
        _delay_ms(250);
        
        LED_ON;
        gamepad_buttons = 0x10;
        usb_gamepad_send();
        _delay_ms(250);
        
        LED_OFF;
        gamepad_buttons = 0x20;
        usb_gamepad_send();
        _delay_ms(250);
        
        LED_ON;
        gamepad_buttons = 0x40;
        usb_gamepad_send();
        _delay_ms(250);
        
        LED_OFF;
        gamepad_buttons = 0x80;
        usb_gamepad_send();
        _delay_ms(250);
        
        LED_ON;
        gamepad_buttons = 0xff;
        usb_gamepad_send();
        _delay_ms(250);
        
        LED_OFF;
        gamepad_buttons = 0x55;
        usb_gamepad_send();
        _delay_ms(250);
        
        LED_ON;
        gamepad_buttons = 0xAA;
        usb_gamepad_send();
        _delay_ms(250);
        
        LED_ON;
        gamepad_buttons = 0x00;
        usb_gamepad_send();
        _delay_ms(250);
        
    }
}

int main(void) {
    setup();
    run();
    return 0;
}


// Timer 0 overflow interrupt handler.
ISR(TIMER0_OVF_vect) {
}
