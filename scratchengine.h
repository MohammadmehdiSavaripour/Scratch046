#ifndef SCRATCH_ENGINE_H
#define SCRATCH_ENGINE_H

#include "scratchdefinition.h"
#include <cmath>
#include <iostream>

using namespace std;

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
    bool autoAdvance = true; 

    switch (current.type) {
        case MOVE_STEPS: {
            double oldX = sprite.x; double oldY = sprite.y;
            double rad = toRadians(sprite.direction);
            sprite.x += current.val1 * cos(rad);
            sprite.y += current.val1 * sin(rad);
            
            if (sprite.isPenDown) {
                sprite.penTrail.push_back({oldX, oldY, sprite.x, sprite.y, sprite.penR, sprite.penG, sprite.penB, sprite.penThickness});
            }
            break;
        }
        case TURN_RIGHT: sprite.direction += current.val1; break;
        case TURN_LEFT:  sprite.direction -= current.val1; break;
        case GO_TO_XY:   sprite.x = current.val1; sprite.y = current.val2; break;
        case PEN_DOWN:   sprite.isPenDown = true; break;
        case PEN_UP:     sprite.isPenDown = false; break;
        case SET_PEN_COLOR: 
            sprite.penR = (Uint8)current.val1; 
            sprite.penG = (Uint8)current.val2; 
            sprite.penB = (Uint8)current.val3; 
            break;

        case REPEAT_START: {
            LoopState newLoop;
            newLoop.startIndex = sprite.currentBlockIndex;
            newLoop.remaining = (int)current.val1;
            newLoop.isForever = false;
            sprite.loopStack.push_back(newLoop);
            break; 
        }

        case REPEAT_END: {
            if (!sprite.loopStack.empty()) {
                LoopState &top = sprite.loopStack.back();
                
                if (!top.isForever) {
                    top.remaining--;
                }

                if (top.isForever || top.remaining > 0) {
                    sprite.currentBlockIndex = top.startIndex + 1;
                    autoAdvance = false; 
                } else {
                    sprite.loopStack.pop_back();
                }
            }
            break;
        }
        
    }

    if (autoAdvance) {
        sprite.currentBlockIndex++;
    }
}

#endif