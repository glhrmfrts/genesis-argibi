#include "../inc/ingame.h"
#include "../inc/global.h"
#include "../res/map_collisiondata.h"
#include "../inc/player.h"
#include "../inc/camera.h"

#define SFX_IMPACT 64

const f16 gravityScale = FIX16(0.5);

Map* map;
Map* map_bg;

void inGameInit() {
    SYS_disableInts();

	u16 ind = TILE_USER_INDEX;

	if (true) {
		VDP_loadTileSet(&map_bg_tileset, ind, DMA);
		PAL_setPalette(BACKGROUND_PALETTE, map_bg_palette.data, DMA);
		map_bg = MAP_create(&map_bg_map, BACKGROUND_PLANE, TILE_ATTR_FULL(BACKGROUND_PALETTE, FALSE, FALSE, FALSE, ind));
		MAP_scrollTo(map_bg, 100, 100);
		ind += map_bg_tileset.numTile;
	}

	VDP_loadTileSet(&map_tileset, ind, DMA);
	PAL_setPalette(LEVEL_PALETTE, map_palette.data, DMA);
	map = MAP_create(&map_map, TILEMAP_PLANE, TILE_ATTR_FULL(LEVEL_PALETTE, FALSE, FALSE, FALSE, ind));
	MAP_scrollTo(map, 100, 100);
	ind += map_tileset.numTile;

	DMA_flushQueue();

	XGM_setPCM(SFX_IMPACT, snd_impact, sizeof(snd_impact));

	// Init collision tiles

	CTILE_init(
		res_map_layer0_data,
		res_map_layer0_len,
		res_map_layer0_width,
		res_map_layer0_height
	);

	// Init player

	playerInit();

	// Init camera

	setupCamera(newVector2D_u16(160, 112), 20, 20);

    SYS_enableInts();
}

//In order to make this data more accessible from all scripts we write them into a struct from global.h
void inGameJoyEvent(u16 joy, u16 changed, u16 state) {
	input.joy = joy;
	input.changed = changed;
	input.state = state;

	if ((changed & BUTTON_A) && (state & BUTTON_A)) {
		//XGM_startPlayPCM(SFX_IMPACT, 15, SOUND_PCM_CH2);
	}

	playerInputChanged();
}

void inGameUpdate() {
	updatePlayer();
	updateCamera();
	SPR_update();
}