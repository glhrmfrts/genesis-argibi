#pragma once

#include <genesis.h>
#include <resources.h>

#define TILEMAP_PLANE BG_A

#define LEVEL_PALETTE  PAL0
#define PLAYER_PALETTE PAL1

// tile values
#define GROUND_TILE 4
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
extern Map* map;