#include "../inc/ingame.h"
#include "../inc/global.h"
#include "../inc/player.h"
#include "../inc/camera.h"
#include "../inc/levels.h"

#define SFX_IMPACT 64

const f16 gravityScale = FIX16(0.5);

Map* map;
Map* map_bg;

int currentLevel = 0;

void loadLevel(int level) {
    LevelDef* ldef = &levels[level];

    u16 ind = TILE_USER_INDEX;

    // load backgrund
    VDP_loadTileSet(ldef->bg.tileset, ind, DMA);
    PAL_setPalette(BACKGROUND_PALETTE, ldef->bg.palette->data, DMA);
    map_bg = MAP_create(ldef->bg.map, BACKGROUND_PLANE, TILE_ATTR_FULL(BACKGROUND_PALETTE, FALSE, FALSE, FALSE, ind));
    ind += ldef->bg.tileset->numTile;

    // load foreground
	VDP_loadTileSet(ldef->fg.tileset, ind, DMA);
	PAL_setPalette(LEVEL_PALETTE, ldef->fg.palette->data, DMA);
	map = MAP_create(ldef->fg.map, FOREGROUND_PLANE, TILE_ATTR_FULL(LEVEL_PALETTE, FALSE, FALSE, FALSE, ind));
	ind += ldef->fg.tileset->numTile;

    // Init collision tiles

	CTILE_init(
		ldef->collision.data,
		ldef->collision.len,
		ldef->collision.width,
		ldef->collision.height
	);
}

void inGameInit() {
    SYS_disableInts();

    loadLevel(currentLevel);

	DMA_flushQueue();

	XGM_setPCM(SFX_IMPACT, snd_impact, sizeof(snd_impact));

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