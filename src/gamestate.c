#include "../inc/gamestate.h"

static GameState* currentState;

void GS_change(GameState* newState) {
	if (currentState && currentState->leaveFunc) {
		currentState->leaveFunc(newState);
	}

	if (newState && newState->enterFunc) {
		newState->enterFunc(currentState);
	}

	currentState = newState;
}

void GS_update() {
	if (currentState && currentState->updateFunc) {
		currentState->updateFunc();
	}
}

void GS_joyHandler(u16 joy, u16 changed, u16 state) {
    if (currentState && currentState->joyFunc) {
        currentState->joyFunc(joy, changed, state);
    }
}