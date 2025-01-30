#pragma once

#include <genesis.h>

struct GameState_s;

typedef void (*GameStateEnterFunc)(struct GameState_s* prev);
typedef void (*GameStateLeaveFunc)(struct GameState_s* new);
typedef void (*GameStateUpdateFunc)(void);
typedef void (*GameStateJoyFunc)(u16 joy, u16 changed, u16 state);

typedef struct GameState_s {
	GameStateEnterFunc enterFunc;
	GameStateLeaveFunc leaveFunc;
	GameStateUpdateFunc updateFunc;
    GameStateJoyFunc joyFunc;
} GameState;

void GS_change(GameState* newState);
void GS_update();
void GS_joyHandler(u16 joy, u16 changed, u16 state);
