#pragma once

#include <genesis.h>

typedef struct cTileData { u16 tileid; u16 idx; } cTileData;

void CTILE_init(const cTileData* tiledata, u32 len, u16 width, u16 height);
void CTILE_free();
u16 CTILE_getTileValue(s16 x, s16 y);