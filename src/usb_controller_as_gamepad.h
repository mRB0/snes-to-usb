#ifndef USB_CONTROLLER_AS_GAMEPAD_
#define USB_CONTROLLER_AS_GAMEPAD_

#include "gamepad.h"
#include "usb_gamepad.h"

class USBControllerAsGamepad : public IUSBController {

public:

    USBControllerAsGamepad() : IUSBController(usb_gamepad1, usb_gamepad2)
        {
        }
    
    virtual void init() {
        usb_gamepad_init();
    }
    
    virtual uint8_t is_configured() {
        return usb_gamepad_configured();
    }
    
    virtual int8_t gamepad1_send() {
        return usb_gamepad1_send();
    }
    
    virtual int8_t gamepad2_send() {
        return usb_gamepad2_send();
    }


};



#endif
