#ifndef SCRATCH_ENGINE_H
#define SCRATCH_ENGINE_H

#include "scratchdefinition.h"
#include <cmath>
#include <iostream>

const int STAGE_WIDTH = 800;
const int STAGE_HEIGHT = 600;
const double PI = 3.14159265358979323846;

double toRadians(double degrees) {
    return degrees * (PI / 180.0);
}

void executeNextBlock(Sprite &sprite) {
    if (!sprite.isRunning || sprite.currentBlockIndex >= sprite.script.size()) {
        sprite.isRunning = false;
        return;
    }

    Block &current = sprite.script[sprite.currentBlockIndex];

    switch (current.type) {
        case MOVE_STEPS: {
            double rad = toRadians(sprite.direction);
            sprite.x += current.val1 * cos(rad);
            sprite.y += current.val1 * sin(rad);
            break;
        }

        case TURN_RIGHT:
            sprite.direction += current.val1;
            break;

        case TURN_LEFT:
            sprite.direction -= current.val1;
            break;

        case GO_TO_XY:
            sprite.x = current.val1;
            sprite.y = current.val2;
            break;

        case IF_ON_EDGE_BOUNCE: {
            bool bounced = false;
            
            if (sprite.x > STAGE_WIDTH) {
                sprite.x = STAGE_WIDTH;
                sprite.direction = 180 - sprite.direction;
                bounced = true;
            }
            else if (sprite.x < 0) {
                sprite.x = 0;
                sprite.direction = 180 - sprite.direction;
                bounced = true;
            }
            
            if (sprite.y > STAGE_HEIGHT) {
                sprite.y = STAGE_HEIGHT;
                sprite.direction = 360 - sprite.direction;
                bounced = true;
            }
            else if (sprite.y < 0) {
                sprite.y = 0;
                sprite.direction = 360 - sprite.direction;
                bounced = true;
            }
            break;
        }
    }

    sprite.currentBlockIndex++;
}

#endif