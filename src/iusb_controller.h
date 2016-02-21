#ifndef IUSB_CONTROLLER_
#define IUSB_CONTROLLER_

#include "gamepad.h"

class IUSBController {

public:

    IUSBController(struct gamepad &gp1, struct gamepad &gp2) :
        gamepad1(gp1), gamepad2(gp2)
        {
    }
    
    virtual void init() = 0;
    virtual uint8_t is_configured() = 0;
    virtual int8_t gamepad1_send() = 0;
    virtual int8_t gamepad2_send() = 0;
    
    virtual void ISR_USB_GEN_vect() = 0;
    virtual void ISR_USB_COM_vect() = 0;

    struct gamepad &gamepad1;
    struct gamepad &gamepad2;

};



#endif
