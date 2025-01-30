#include "global.h"
#include "player.h"
#include "map_collisiondata.h"
#include "../inc/gamestate.h"
#include "../inc/ingame.h"
#include "../inc/start.h"


GameState startState;
GameState inGameState;


int main()
{
	SPR_init();
	
	JOY_setEventHandler(GS_joyHandler);

	startState.enterFunc = startInit;
	startState.updateFunc = startUpdate;
	startState.joyFunc = startJoyEvent;

	inGameState.enterFunc = inGameInit;
	inGameState.updateFunc = inGameUpdate;
	inGameState.joyFunc = inGameJoyEvent;

	GS_change(&startState);

	while (TRUE) {
		GS_update();
		SYS_doVBlankProcess();
	}

	return (0);
}
