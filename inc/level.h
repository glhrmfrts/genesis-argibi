#ifndef RGB_LEVEL_H
#define RGB_LEVEL_H

#include <genesis.h>

#define MAX_BLOCKS 512

/*
    Map cell data:
    Bits: 7 --- 6 --- 5 --- 4 --- 3 --- 2 --- 1 --- 0
                                  ^   Block type    ^
          ^  Block specific data  ^
*/

#define CELL_REGULAR_BLOCK 0x01
#define CELL_CHANGING_BLOCK 0x02

#define CELL_MOVING_BLOCK_X 0x03
#define CELL_MOVING_BLOCK_Y 0x04

#define CELL_PUSH_BLOCK 0x05
#define CELL_LAVA_BLOCK 0x0e
#define CELL_EXIT_BLOCK 0x0f
#define CELL_START_BLOCK 0xff

// Upper bits for the regular and changing block
#define CELL_COLOR_RED 0x10
#define CELL_COLOR_GREEN 0x20
#define CELL_COLOR_BLUE 0x40
#define CELL_COLOR_GREY 0x00


typedef struct RegularBlock_s { int dummy; } RegularBlock;

typedef struct BlockEntry_s
{
  u16 type;
  u16 x;
  u16 y;
  u16 color;
  u16 cosmetic_color;
  union {
    RegularBlock block;
  };
} BlockEntry;


int level_width;
int level_height;
int level_start_x;
int level_start_y;
int level_n_blocks;
int level_timer;
int level_exit_image;
int level_initialized;
BlockEntry* blocks;


void Level_init()
{
  blocks = (BlockEntry*)MEM_alloc(MAX_BLOCKS * sizeof(BlockEntry));
}

void Level_deinit()
{
  MEM_free(blocks);
}

BlockEntry* Level_createRegularBlock(int x, int y, int color)
{
  BlockEntry* block = blocks + level_n_blocks;
  block->type = CELL_REGULAR_BLOCK;
  block->x = x;
  block->y = y;
  block->color = color;
  level_n_blocks++;
  return block;
}

BlockEntry* Level_createExitBlock(int x, int y)
{
  BlockEntry* block = blocks + level_n_blocks;
  block->type = CELL_EXIT_BLOCK;
  block->x = x;
  block->y = y;
  block->color = CELL_COLOR_GREEN;
  level_n_blocks++;
  return block;
}

static void processMapCell(int x, int y, int cell)
{
  int blockType = cell & 0x0f;
  switch (blockType) {
    case CELL_START_BLOCK:
      level_start_x = x;
      level_start_y = y;
      break;
    case CELL_REGULAR_BLOCK:
      Level_createRegularBlock(x, y, cell & 0xf0);
      break;
  }
}

static int getMapIndex(int x, int y) { return y*level_width + x; }

void Level_create(u8* mapdata, int w, int h) {
  level_width = w;
  level_height = h;
  level_n_blocks = 0;
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      processMapCell(x, y, mapdata[getMapIndex(x, y)]);
    }
  }
}

#endif