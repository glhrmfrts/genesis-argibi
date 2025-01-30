#pragma once

#include <genesis.h>

#include "types.h"

extern Vect2D_s16 cameraPosition;

void setupCamera(Vect2D_u16 deadZoneCenter, u16 deadZoneWidth, u16 deadZoneHeight);
void updateCamera();