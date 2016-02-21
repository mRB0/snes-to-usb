#ifndef gamepad_h_
#define gamepad_h_

#ifdef __cplusplus
extern "C" {
#endif
    
#include <stdint.h>

struct gamepad {
    volatile int8_t x_axis;
    volatile int8_t y_axis;
    volatile uint8_t buttons;
};

#ifdef __cplusplus
}
#endif

#endif
