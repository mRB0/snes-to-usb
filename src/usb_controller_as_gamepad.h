#ifndef USB_CONTROLLER_AS_GAMEPAD_
#define USB_CONTROLLER_AS_GAMEPAD_

#include "gamepad.h"
#include "usb_gamepad.h"
#include "iusb_controller.h"

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
    
    virtual int8_t send() {
        int8_t result;
        
        result = usb_gamepad1_send();
        result |= usb_gamepad2_send();
        
        return result;
    }

    virtual void ISR_USB_GEN_vect() {
        usb_gamepad_ISR_USB_GEN_vect();
    }
    
    virtual void ISR_USB_COM_vect() {
        usb_gamepad_ISR_USB_COM_vect();
    }

};



#endif
