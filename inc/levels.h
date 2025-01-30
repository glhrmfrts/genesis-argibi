#pragma once

#include <genesis.h>
#include "collisiontile.h"

typedef struct {
  TileSet* tileset;
  Palette* palette;
  MapDefinition* map;
} LevelTileMapDef;

typedef struct {
  cTileData* data;
  u32 len;
  u16 width;
  u16 height;  
} LevelCollisionDef;

typedef struct {
  LevelCollisionDef collision;
  LevelTileMapDef fg;
  LevelTileMapDef bg;
} LevelDef;

extern LevelDef levels[32];
