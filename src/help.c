#include "help.h"
#include "gamestate.h"
#include "global.h"

void helpInit() {
  VDP_clearPlane(BG_A, true);

  VDP_drawText("YOU DON'T COLLIDE WITH", 10, 12);
  VDP_drawText("ANYTHING OF THE SAME", 10, 13);
  VDP_drawText("COLOR AS YOU", 10, 14);
}

void helpUpdate() {

}

void helpJoyEvent(u16 joy, u16 changed, u16 state) {
  if ((changed & BUTTON_START) && (state & BUTTON_START)) {
    GS_change(&menuState);
  }
}