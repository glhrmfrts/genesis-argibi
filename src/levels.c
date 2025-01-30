#include "../inc/levels.h"
#include "../res/resources.h"
#include "../res/map_collisiondata.h"
#include "../res/map2_collisiondata.h"

LevelDef levels[32] = {
    {
        .fg = {
            .map = &map_map,
            .palette = &map_palette,
            .tileset = &map_tileset,
        },
        .bg = {
            .map = &map_bg_map,
            .palette = &map_bg_palette,
            .tileset = &map_bg_tileset,
        },
        .collision = {
            .data = res_map_layer1_data,
            .len = res_map_layer1_len,
            .width = res_map_layer1_width,
            .height = res_map_layer1_height,
        }
    },
    {
        .fg = {
            .map = &map2_map,
            .palette = &map2_palette,
            .tileset = &map2_tileset,
        },
        .bg = {
            .map = &map2_bg_map,
            .palette = &map2_bg_palette,
            .tileset = &map2_bg_tileset,
        },
        .collision = {
            .data = res_map2_layer1_data,
            .len = res_map2_layer1_len,
            .width = res_map2_layer1_width,
            .height = res_map2_layer1_height,
        }
    },
    {0}
};

