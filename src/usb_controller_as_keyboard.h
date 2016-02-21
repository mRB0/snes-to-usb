#ifndef USB_CONTROLLER_AS_KEYBOARD_
#define USB_CONTROLLER_AS_KEYBOARD_

#include "gamepad.h"
#include "usb_keyboard_gamepad.h"
#include "iusb_controller.h"

class USBControllerAsKeyboard : public IUSBController {

public:

    USBControllerAsKeyboard() : IUSBController(kbpad1, kbpad2)
        {
        }
    
    virtual void init() {
        usb_kbpad_init();
    }
    
    virtual uint8_t is_configured() {
        return usb_kbpad_configured();
    }
    
    virtual int8_t send() {
        return usb_kbpad_send();
    }

    virtual void ISR_USB_GEN_vect() {
        usb_kbpad_ISR_USB_GEN_vect();
    }
    
    virtual void ISR_USB_COM_vect() {
        usb_kbpad_ISR_USB_COM_vect();
    }

};



#endif
