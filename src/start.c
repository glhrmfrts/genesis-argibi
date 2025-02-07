#include "../inc/start.h"
#include "../inc/global.h"

void startInit() {

    SYS_disableInts();
    VDP_drawText("Press start", 10, 10);
    SYS_enableInts();

}

void startUpdate() {

}

void startJoyEvent(u16 joy, u16 changed, u16 state) {
    if (joy == JOY_1) {
        if ((changed & BUTTON_START) && (state & BUTTON_START)) {
            GS_change(&menuState);
        }
    }
}