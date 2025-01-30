#pragma once

#include <genesis.h>
#include <resources.h>
#include "gamestate.h"

#define TILEMAP_PLANE BG_A
#define BACKGROUND_PLANE BG_B

#define LEVEL_PALETTE  			PAL0
#define BACKGROUND_PALETTE 	PAL1
#define PLAYER_PALETTE 			PAL2

// tile values
#define GROUND_TILE 4
#define START_TILE 	50
#define EXIT_TILE   18
#define LADDER_TILE 19
#define ONE_WAY_PLATFORM_TILE 20

// Gameplay colors
#define COL_RED 0
#define COL_GREEN 1
#define COL_BLUE 2

struct {
	u16 joy;
	u16 changed;
	u16 state;
}input;

extern const f16 gravityScale;

extern AABB playerBounds;

extern AABB mapSize;
extern Vect2D_u16 mapTileSize;
extern Vect2D_u16 mapStartTilePos;
extern Map* map;
extern Map* map_bg;

extern GameState startState;
extern GameState inGameState;