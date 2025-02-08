#include "global.h"
#include "player.h"
#include "map_collisiondata.h"
#include "gamestate.h"
#include "ingame.h"
#include "start.h"
#include "menu.h"
#include "help.h"

// stb
#ifndef STB_SPRINTF_IMPLEMENTATION
#define STB_SPRINTF_IMPLEMENTATION
#endif
#ifndef STB_SPRINTF_NOSTD
#define STB_SPRINTF_NOSTD
#endif
#ifndef STB_SPRINTF_NOUNALIGNED
#define STB_SPRINTF_NOUNALIGNED
#endif

#include "ext/stb/stb_sprintf.h"


GameState startState;
GameState menuState;
GameState inGameState;
GameState helpState;


int main()
{
	JOY_init();

	CON_setVsnprintf(stbsp_vsnprintf);
	CON_setTransferMethod(DMA_QUEUE);
	VDP_setTextPlane(BG_A);

	SPR_init();

	JOY_setEventHandler(GS_joyHandler);

	startState.enterFunc = startInit;
	startState.updateFunc = startUpdate;
	startState.joyFunc = startJoyEvent;

	menuState.enterFunc = menuInit;
	menuState.updateFunc = menuUpdate;
	menuState.joyFunc = menuJoyEvent;

	inGameState.enterFunc = inGameInit;
	inGameState.updateFunc = inGameUpdate;
	inGameState.joyFunc = inGameJoyEvent;

	helpState.enterFunc = helpInit;
	helpState.updateFunc = helpUpdate;
	helpState.joyFunc = helpJoyEvent;

	GS_change(&menuState);

	while (TRUE) {
		GS_update();
		SYS_doVBlankProcess();
	}

	return (0);
}
