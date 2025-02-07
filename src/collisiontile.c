#include "collisiontile.h"
#include "global.h"
#include "../inc/types.h"

//Dynamic 2D array where we store the collision map data
//We could read that directly from ROM but in the long term it's cleaner and/or more performant
static u16* currentMap = NULL;

//Downlscaled size of the map in order to match the collision map size
Vect2D_u16 mapTileSize;
AABB mapSize;
Vect2D_u16 mapStartTilePos;

void CTILE_free() {
	//We have to free the collision map data in this way in order to avoid memory leaks
	if (currentMap) {
		MEM_free(currentMap);
		currentMap = NULL;
	}
}

bool SEEKING = false;

void CTILE_init(const cTileData* tiledata, u32 len, u16 width, u16 height) {
	CTILE_free();

	// Each tile is 16x16 px so shift up by 4 to get pixel size

	mapTileSize = newVector2D_u16(width, height);
	mapSize = newAABB(0, width << 4, 0, height << 4);

  // Fill all with 0 then copy tiles from tiledata

	u32 bytesize = width * height * sizeof(u16);
	currentMap = (u16*)MEM_alloc(bytesize);
  memset(currentMap, 0, bytesize);

  for (u32 i = 0; i < len; i++) {
		const cTileData* t = tiledata + i;
    currentMap[t->idx] = t->tileid;
		if (t->tileid == START_TILE) {
			mapStartTilePos.x = t->idx % width;
			mapStartTilePos.y = t->idx / width;
		}
  }
}

u16 CTILE_getTileValue(s16 x, s16 y) {
	if (x >= mapTileSize.x || x < 0 || y < 0 || y >= mapTileSize.y)
		return 0;
	//If the position is inside the collision map return the value of the tile from it
	return currentMap[y * mapTileSize.x + x];
}