#include "global.h"
#include "map.h"
#include "player.h"
#include "physics.h"
#include "collisiontile.h"

#define NUM_ANIM 3

#define COL_ANIM_OFS_RED 		0
#define COL_ANIM_OFS_GREEN 	1*NUM_ANIM
#define COL_ANIM_OFS_BLUE   2*NUM_ANIM

#define ANIM_IDLE 0
#define ANIM_WALK 1
#define ANIM_FALL 2

struct pBody playerBody;

void checkCollisions();
void updateAnimations();

const s16 coyoteTime = 10;
s16 currentCoyoteTime;
const s16 jumpBufferTime = 10;
s16 currentJumpBufferTime;

bool collidingAgainstStair;
bool runningAnim;

u16 dyingSteps;
const u16 dieDelay = 10;

const u16 oneWayPlatformErrorCorrection = 5;

s16 stairLeftEdge;
const u16 stairPositionOffset = 4;

const int jumpButton = BUTTON_A;
const int colorButton = BUTTON_B;
bool playerChangedColor = false;
int nonColoredAnimation = ANIM_IDLE;

//Player collider bounds position
AABB playerBounds;

void playerInit() {
	//Create the sprite and palette for the player

	playerBody.sprite = SPR_addSprite(
    &player_sprite,
    mapStartTilePos.x << 4, mapStartTilePos.y << 4,
    TILE_ATTR(PLAYER_PALETTE, false, false, false));

	PAL_setPalette(PLAYER_PALETTE, player_sprite.palette->data, DMA);

	//Set the global position of the player, the local position will be updated once we are in the main loop
	playerBody.globalPosition.x = mapStartTilePos.x << 4;
	playerBody.globalPosition.y = mapStartTilePos.y << 4;

	//We set collider size of the player relative to the sprite top-left
	playerBody.aabb = newAABB(16, 48, 4, 64);
	//This collider is thinner because if the width is >=16 it could interfere with the lateral walls
	playerBody.climbingStairAABB = newAABB(26, 38, playerBody.aabb.min.y, playerBody.aabb.max.y);

	//Calculate where's the center point of the player
	playerBody.centerOffset.x = ((playerBody.aabb.min.x + playerBody.aabb.max.x) >> 1);
	playerBody.centerOffset.y = ((playerBody.aabb.min.y + playerBody.aabb.max.y) >> 1);

	// Default movement values
	playerBody.speed = 2;
	playerBody.climbingSpeed = 1;
	playerBody.maxFallSpeed = 6;
	playerBody.jumpSpeed = 7;
	playerBody.facingDirection = 1;
	playerBody.acceleration = FIX16(.25);
	playerBody.deceleration = FIX16(.2);

	playerBody.color = COL_RED;

	//Setup the jump SFX with an index between 64 and 255
	// XGM_setPCM(64, jump, sizeof(jump));
}

fix16 gravityFactor = FIX16(1.0);

void playerInputChanged() {
	u16 joy = input.joy;
	u16 state = input.state;
	u16 changed = input.changed;

	//We only read data from the joypad 1
	if (joy == JOY_1) {
		//Update x velocity
		if (state & BUTTON_RIGHT) {
			playerBody.input.x = 1;
		}else if (state & BUTTON_LEFT) {
			playerBody.input.x = -1;
		}else if ((changed & BUTTON_RIGHT) | (changed & BUTTON_LEFT)) {
			playerBody.input.x = 0;
		}

		//Jump button via jumpbuffer
		//Also used to stop climbing the stairs
		if (changed & jumpButton) {
			if (state & jumpButton) {
				if (playerBody.climbingStair) {
					playerBody.climbingStair = false;
				}else {
					currentJumpBufferTime = jumpBufferTime;
				}
			}else if (playerBody.jumping && playerBody.velocity.fixY < 0) {
				//If the button is released we remove half of the velocity
				playerBody.velocity.fixY = fix16Mul(playerBody.velocity.fixY, FIX16(.5));
			}
		}

		if (changed & colorButton) {
			if (state & colorButton) {
				playerBody.color += 1;
				if (playerBody.color > COL_BLUE) {
					playerBody.color = COL_RED;
				}
				playerChangedColor = true;
			}
		}

		//Down and up buttons are only used when it is climbing stair
		//NOTE: Up direction is -1 and down direction is 1, this is because the Mega Drive draws the screen from top to bottom
		if (changed & BUTTON_DOWN) {
			if (state & BUTTON_DOWN) {

				gravityFactor = FIX16(1.0);

				playerBody.input.y = 1;
				if (playerBody.climbingStair) {
					playerBody.velocity.fixY = FIX16(playerBody.climbingSpeed);
				}else if (playerBody.onStair) {
					playerBody.velocity.fixY = FIX16(playerBody.climbingSpeed);
					playerBody.climbingStair = true;
				}
			}else {
				playerBody.input.y = 0;
				if (playerBody.climbingStair) {
					playerBody.velocity.fixY = 0;
				}
			}
		}
		if (changed & BUTTON_UP) {
			if (state & BUTTON_UP) {

				gravityFactor = FIX16(0.0);

				playerBody.input.y = -1;
				if (collidingAgainstStair && !playerBody.onStair) {
					playerBody.climbingStair = true;
					playerBody.velocity.fixY = FIX16(-playerBody.climbingSpeed);
				}
			}else {
				playerBody.input.y = 0;
				if (playerBody.climbingStair) {
					playerBody.velocity.fixY = 0;
				}
			}
		}
	}
}

void updatePlayer() {
	//Check if the player wants to climb a stair
	if(collidingAgainstStair && ((playerBody.onStair && playerBody.input.y > 0) || (!playerBody.onStair && playerBody.input.y < 0))){
		playerBody.climbingStair = true;
		playerBody.velocity.fixY = FIX16(playerBody.climbingSpeed * playerBody.input.y);
	}

	//Check if player wants to jump by looking the coyote time and jump buffer
	if (currentCoyoteTime > 0 && currentJumpBufferTime > 0) {
		playerBody.jumping = true;
		//Play the SFX with the index 64 (jump sfx) with the highest priority
		XGM_startPlayPCM(64, 15, SOUND_PCM_CH2);
		playerBody.velocity.fixY = FIX16(-playerBody.jumpSpeed);

		currentCoyoteTime = 0;
		currentJumpBufferTime = 0;
	}

	//The ground hasn't been checked yet so we only decrease the jump buffer time for now
	currentJumpBufferTime = clamp((currentJumpBufferTime - 1), 0, jumpBufferTime); //Clamp to avoid underflowing, it is unlikely to happen but can happen

	//If the player is climbing a stair, it only needs to go upward, if not, we apply horizontal movement
	if (playerBody.climbingStair) {
		playerBody.velocity.x = playerBody.velocity.fixX = 0;
		playerBody.globalPosition.x = stairLeftEdge - stairPositionOffset;
	}else {
		if (playerBody.input.x > 0) {
			if (playerBody.velocity.x != playerBody.speed)
				playerBody.velocity.fixX += playerBody.acceleration;
		}else if (playerBody.input.x < 0) {
			if (playerBody.velocity.x != -playerBody.speed)
				playerBody.velocity.fixX -= playerBody.acceleration;
		}else if (playerBody.onGround) {
			if (playerBody.velocity.x > 0)
				playerBody.velocity.fixX -= playerBody.deceleration;
			else if (playerBody.velocity.x < 0)
				playerBody.velocity.fixX += playerBody.deceleration;
			else
				playerBody.velocity.fixX = 0;
		}
		playerBody.velocity.x = clamp(fix16ToInt(playerBody.velocity.fixX), -playerBody.speed, playerBody.speed);
	}

	//Apply gravity with a terminal velocity
	if (!playerBody.onGround && !playerBody.climbingStair) {
		if (fix16ToInt(playerBody.velocity.fixY) <= playerBody.maxFallSpeed) {
			playerBody.velocity.fixY = playerBody.velocity.fixY + fix16Mul(gravityScale, gravityFactor);
		}else {
			playerBody.velocity.fixY = FIX16(playerBody.maxFallSpeed);
		}
	}

	//Once all the input-related have been calculated, we apply the velocities to the global positions
	playerBody.globalPosition.x += playerBody.velocity.x;
	playerBody.globalPosition.y += fix16ToInt(playerBody.velocity.fixY);

	//Now we can check for collisions and correct those positions
	checkCollisions();

	//Now that the collisions have been checked, we know if the player is on a stair or not
	if (!collidingAgainstStair && playerBody.climbingStair) {
		playerBody.climbingStair = false;
		playerBody.input.y = 0;
	}

	//Once the positions are correct, we position the player taking into account the camera position
	playerBody.position.x = playerBody.globalPosition.x - cameraPosition.x;
	playerBody.position.y = playerBody.globalPosition.y - cameraPosition.y;
	SPR_setPosition(playerBody.sprite, playerBody.position.x, playerBody.position.y);

	//Update the player animations
	updateAnimations();

	//Reset when falling off the screen
	if (playerBody.falling) {
		dyingSteps++;
		if(dyingSteps > dieDelay){
			//SYS_reset();
		}
	}
}

static s16 getColoredAnimationIndex(s16 anim) {
	return anim + playerBody.color*NUM_ANIM;
}

static void setPlayerAnimation(s16 anim) {
	SPR_setAnim(playerBody.sprite, getColoredAnimationIndex(anim));
	nonColoredAnimation = anim;
}

void updateAnimations() {
	// Sprite flip depending on the horizontal input
	if (playerBody.input.x > 0) {
		SPR_setHFlip(playerBody.sprite, false);
		playerBody.facingDirection = 1;
	}else if (playerBody.input.x < 0) {
		SPR_setHFlip(playerBody.sprite, true);
		playerBody.facingDirection = -1;
	}

	//If the player is on ground and not climbing the stair it can be idle or running
	if (playerBody.velocity.fixY == 0 && !playerBody.climbingStair) {
		if (playerBody.velocity.x != 0 && runningAnim == false && playerBody.onGround) {
			setPlayerAnimation(ANIM_WALK);
			runningAnim = true;
		}else if (playerBody.velocity.x == 0 && playerBody.onGround) {
			setPlayerAnimation(ANIM_IDLE);
			runningAnim = false;
		}
	}

	if (!playerBody.onGround && fix16ToInt(playerBody.velocity.fixY) > 1) {
		setPlayerAnimation(ANIM_FALL);
		runningAnim = false;
	}

	//Climb animation
	if (playerBody.climbingStair) {
		setPlayerAnimation(ANIM_FALL);
	}

// Force update the animation if the player changed color
	if (playerChangedColor) {
		setPlayerAnimation(nonColoredAnimation);
	}
}

static bool playerShouldCollideWithTile(u16 tileValue) {
	// Player can collide with ground or with tiles of different color from their own
	if (tileValue >= RED_TILE && tileValue <= GROUND_TILE) {
		return (tileValue == GROUND_TILE) || (playerBody.color+1 != tileValue);
	}
	return false;
}

void checkCollisions() {
	//As we now have to check for collisions, we will later check if it is true or false, but for now it is false
	collidingAgainstStair = false;

	//Create level limits
	AABB levelLimits = mapSize;

	//Easy access to the bounds in global pos
	if (playerBody.climbingStair) {
		playerBounds = newAABB(
			playerBody.globalPosition.x + playerBody.climbingStairAABB.min.x,
			playerBody.globalPosition.x + playerBody.climbingStairAABB.max.x,
			playerBody.globalPosition.y + playerBody.climbingStairAABB.min.y,
			playerBody.globalPosition.y + playerBody.climbingStairAABB.max.y
		);
	}else {
		playerBounds = newAABB(
			playerBody.globalPosition.x + playerBody.aabb.min.x,
			playerBody.globalPosition.x + playerBody.aabb.max.x,
			playerBody.globalPosition.y + playerBody.aabb.min.y,
			playerBody.globalPosition.y + playerBody.aabb.max.y
		);
	}

	//We can see this variables as a way to avoid thinking that a ground tile is a wall tile
	//Skin width (yIntVelocity) changes depending on the vertical velocity
	s16 yIntVelocity = fix16ToRoundedInt(playerBody.velocity.fixY);
	s16 playerHeadPos = playerBody.aabb.min.y - yIntVelocity + playerBody.globalPosition.y;
	s16 playerFeetPos = playerBody.aabb.max.y - yIntVelocity + playerBody.globalPosition.y;

	//Positions in tiles
	Vect2D_u16 minTilePos = posToTile(newVector2D_s16(playerBounds.min.x, playerBounds.min.y));
	Vect2D_u16 maxTilePos = posToTile(newVector2D_s16(playerBounds.max.x, playerBounds.max.y));

	//Used to limit how many tiles we have to check for collision
	Vect2D_u16 tileBoundDifference = newVector2D_u16(maxTilePos.x - minTilePos.x, maxTilePos.y - minTilePos.y);

	//First we check for horizontal collisions
	for (u16 i = 0; i <= tileBoundDifference.y; i++) {
		// Height position constant as a helper
		const u16 y = minTilePos.y + i;

		// Right position constant as a helper
		const u16 rx = maxTilePos.x;

		bool isExit = false;

		u16 rTileValue = CTILE_getTileValue(rx, y);
		//After getting the tile value, we check if that is one of whom we can collide/trigger with horizontally
		if (playerShouldCollideWithTile(rTileValue)) {
			AABB tileBounds = getTileBounds(rx, y);
			//Before taking that tile as a wall, we have to check if that is within the player hitbox, e.g. not seeing ground as a wall
			if (tileBounds.min.x < levelLimits.max.x && tileBounds.min.y < playerFeetPos && tileBounds.max.y > playerHeadPos) {
				levelLimits.max.x = tileBounds.min.x;
				break;
			}
		} else if (rTileValue == LADDER_TILE) {
			stairLeftEdge = getTileLeftEdge(rx);
			collidingAgainstStair = true;
		} else if (rTileValue == EXIT_TILE) {
			isExit = true;
		}

		//Left position constant as a helper
		const s16 lx = minTilePos.x;

		u16 lTileValue = CTILE_getTileValue(lx, y);
		//We do the same here but for the left side
		if (playerShouldCollideWithTile(lTileValue)) {
			AABB tileBounds = getTileBounds(lx, y);
			if (tileBounds.max.x > levelLimits.min.x && tileBounds.min.y < playerFeetPos && tileBounds.max.y > playerHeadPos) {
				levelLimits.min.x = tileBounds.max.x;
				break;
			}
		} else if (lTileValue == LADDER_TILE) {
			stairLeftEdge = getTileLeftEdge(lx);
			collidingAgainstStair = true;
		} else if (lTileValue == EXIT_TILE) {
			isExit = true;
		}

		if (isExit && (playerBody.color+1 != GREEN_TILE)) {
			SYS_reset();
		}
	}

	//After checking for horizontal positions we can modify the positions if the player is colliding
	if (levelLimits.min.x > playerBounds.min.x) {
		playerBody.globalPosition.x = levelLimits.min.x - playerBody.aabb.min.x;
		playerBody.velocity.x = playerBody.velocity.fixX = 0;
	}
	if (levelLimits.max.x < playerBounds.max.x) {
		playerBody.globalPosition.x = levelLimits.max.x - playerBody.aabb.max.x;
		playerBody.velocity.x = playerBody.velocity.fixX = 0;
	}

	//Then, we modify the player position so we can use them to check for vertical collisions
	if (playerBody.climbingStair) {
		playerBounds = newAABB(
			playerBody.globalPosition.x + playerBody.climbingStairAABB.min.x,
			playerBody.globalPosition.x + playerBody.climbingStairAABB.max.x,
			playerBody.globalPosition.y + playerBody.climbingStairAABB.min.y,
			playerBody.globalPosition.y + playerBody.climbingStairAABB.max.y
		);
	}else {
		playerBounds = newAABB(
			playerBody.globalPosition.x + playerBody.aabb.min.x,
			playerBody.globalPosition.x + playerBody.aabb.max.x,
			playerBody.globalPosition.y + playerBody.aabb.min.y,
			playerBody.globalPosition.y + playerBody.aabb.max.y
		);
	}

	//And do the same to the variables that are used to check for them
	minTilePos = posToTile(newVector2D_s16(playerBounds.min.x, playerBounds.min.y));
	maxTilePos = posToTile(newVector2D_s16(playerBounds.max.x - 1, playerBounds.max.y));
	tileBoundDifference = newVector2D_u16(maxTilePos.x - minTilePos.x, maxTilePos.y - minTilePos.y);

	bool onStair = false;

	//To avoid having troubles with player snapping to ground ignoring the upward velocity, we separate top and bottom collisions depending on the velocity
	if (yIntVelocity >= 0) {
		for (u16 i = 0; i <= tileBoundDifference.x; i++) {
			s16 x = minTilePos.x + i;
			u16 y = maxTilePos.y;

			//This is the exact same method that we use for horizontal collisions
			u16 bottomTileValue = CTILE_getTileValue(x, y);

#if 0
			static char str[20];

			intToStr((s16)bottomTileValue, str, 2);
			str[2] = ' ';
			intToStr(x, str + 3, 2);
			str[5] = ' ';
			intToStr(y, str + 6, 2);

			VDP_drawText(str, 5, 5);
#endif

			if (bottomTileValue == GROUND_TILE || bottomTileValue == ONE_WAY_PLATFORM_TILE) {
				if (getTileRightEdge(x) == levelLimits.min.x || getTileLeftEdge(x) == levelLimits.max.x)
					continue;

				u16 bottomEdgePos = getTileTopEdge(y);
				//The error correction is used to add some extra width pixels in case the player isn't high enough by just some of them
				if (bottomEdgePos < levelLimits.max.y && bottomEdgePos >= (playerFeetPos - oneWayPlatformErrorCorrection)) {
					levelLimits.max.y = bottomEdgePos;
				}
			}else if (bottomTileValue == LADDER_TILE) {
				stairLeftEdge = getTileLeftEdge(x);

				u16 bottomEdgePos = getTileTopEdge(y);
				//Only in this case we check for ladder collisions, as we need them to climb them down
				if (bottomEdgePos <= levelLimits.max.y && bottomEdgePos >= playerFeetPos && !playerBody.climbingStair && CTILE_getTileValue(x, y - 1) != LADDER_TILE) {
					collidingAgainstStair = true;
					onStair = true;
					levelLimits.max.y = bottomEdgePos;
					break;
				}
			} else if (bottomTileValue == EXIT_TILE) {
				SYS_reset();
			} else {
				//VDP_clearText(5, 5, 10);
			}
		}
	}else {
		for (u16 i = 0; i <= tileBoundDifference.x; i++) {
			s16 x = minTilePos.x + i;
			u16 y = minTilePos.y;

			//And the same once again
			u16 topTileValue = CTILE_getTileValue(x, y);
			if (topTileValue == GROUND_TILE) {
				if (getTileRightEdge(x) == levelLimits.min.x || getTileLeftEdge(x) == levelLimits.max.x)
					continue;

				u16 upperEdgePos = getTileBottomEdge(y);
				if (upperEdgePos < levelLimits.max.y) {
					levelLimits.min.y = upperEdgePos;
					break;
				}
			}else if (topTileValue == LADDER_TILE) {
				stairLeftEdge = getTileLeftEdge(x);
				collidingAgainstStair = true;
			} else if (topTileValue == EXIT_TILE) {
				SYS_reset();
			}
		}
	}

	//Now we modify the player position and some properties if necessary
	if (levelLimits.min.y > playerBounds.min.y) {
		playerBody.globalPosition.y = levelLimits.min.y - playerBody.aabb.min.y;
		playerBody.velocity.fixY = 0;
	}
	if (levelLimits.max.y <= playerBounds.max.y) {
		if (levelLimits.max.y == 768) {
			playerBody.falling = true;
		}else {
			playerBody.onStair = onStair;
			playerBody.onGround = true;
			playerBody.climbingStair = false;
			currentCoyoteTime = coyoteTime;
			playerBody.jumping = false;
			playerBody.globalPosition.y = levelLimits.max.y - playerBody.aabb.max.y;
			playerBody.velocity.fixY = 0;
		}
	}else {
		playerBody.onStair = playerBody.onGround = false;
		currentCoyoteTime--;
	}
	//This time we don't need to update the playerBounds as they will be updated at the beginning of the function the next frame
}