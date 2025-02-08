#include "../inc/start.h"
#include "../inc/global.h"

#define TEXT_COLUMN 14
#define TEXT_ROW 12
#define NUM_ITEMS 3

s16 cursorPos;
s16 cursorOldPos;
s16 selectedItem;

static void drawCursor() {
    // Clear the old cursor
    VDP_clearText(TEXT_COLUMN - 3, cursorOldPos, 2);

    // Draw the cursor at the new position
    VDP_drawText("->", TEXT_COLUMN - 3, cursorPos);
}

static void selectItem(s16 i) {
    if (i < 0) {
        i = NUM_ITEMS - 1;
    }
    else if (i > NUM_ITEMS - 1) {
        i = 0;
    }
    selectedItem = i;
    cursorOldPos = cursorPos;
    cursorPos = TEXT_ROW + i * 2;
}

void menuInit() {

    VDP_clearPlane(BG_A, true);

    PAL_setPalette(PAL3, logo_image.palette->data, DMA);
    VDP_drawImageEx(BG_B, &logo_image, TILE_ATTR_FULL(PAL3, 0,0,0, TILE_USER_INDEX), 10, 0, false, true);

    VDP_drawText("NEW GAME", TEXT_COLUMN, TEXT_ROW);
    VDP_drawText("PASSWORD", TEXT_COLUMN, TEXT_ROW+2);
    VDP_drawText("HELP", TEXT_COLUMN, TEXT_ROW+4);

    selectItem(0);

    drawCursor();

}

void menuUpdate() {

}

void menuJoyEvent(u16 joy, u16 changed, u16 state) {
    if (joy == JOY_1) {

        if ((changed & BUTTON_DOWN) && (state & BUTTON_DOWN)) {
            selectItem(selectedItem + 1);
            drawCursor();
        }

        if ((changed & BUTTON_UP) && (state & BUTTON_UP)) {
            selectItem(selectedItem - 1);
            drawCursor();
        }

        if ((changed & BUTTON_START) && (state & BUTTON_START)) {
            switch (selectedItem) {
            case 0:
                GS_change(&inGameState);
                break;
            case 1:
                break;
            case 2:
                GS_change(&helpState);
                break;
            }
        }

    }
}