#include "global.h"
#include "player.h"
#include "map_collisiondata.h"

const f16 gravityScale = FIX16(0.5);

Map* map;

void inGameJoyEvent(u16 joy, u16 changed, u16 state);


#define SFX_IMPACT 64


int main()
{
	SPR_init();

	u16 ind = TILE_USER_INDEX;
	VDP_loadTileSet(&map_tileset, ind, DMA);
	PAL_setPalette(LEVEL_PALETTE, map_palette.data, DMA);
	map = MAP_create(&map_map, TILEMAP_PLANE, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, ind));
	MAP_scrollTo(map, 100, 100);

	JOY_setEventHandler(inGameJoyEvent);

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

	while (TRUE) {
		updatePlayer();
		updateCamera();
		SPR_update();
		SYS_doVBlankProcess();
	}

	return (0);
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
