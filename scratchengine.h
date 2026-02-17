#ifndef SCRATCH_ENGINE_H
#define SCRATCH_ENGINE_H
#include "scratchdefinition.h"
#include <cmath>
#include <iostream>
#include <SDL2/SDL.h>
using namespace std;

const int STAGE_WIDTH = 800;
const int STAGE_HEIGHT = 600;
const double PI = 3.14159265358979323846;

double toRadians(double degrees) {
    return degrees * (PI / 180.0);
}

bool checkCondition(Sprite &sprite, ConditionType type, double value) {
    switch (type) {
        case KEY_SPACE_PRESSED: {
            const Uint8 *state = SDL_GetKeyboardState(NULL);
            return state[SDL_SCANCODE_SPACE];
        }
        case MOUSE_CLICKED: {
            int x, y;
            Uint32 buttons = SDL_GetMouseState(&x, &y);
            return (buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
        }
        case TOUCHING_EDGE: {
            return (sprite.x <= 0 || sprite.x >= STAGE_WIDTH || 
                    sprite.y <= 0 || sprite.y >= STAGE_HEIGHT);
        }
        case X_GREATER_THAN:
            return sprite.x > value;
        case X_LESS_THAN:
            return sprite.x < value;
    }
    return false;
}

int findMatchingEnd(Sprite &sprite, int startIndex) {
    int depth = 0;
    for (int i = startIndex + 1; i < sprite.script.size(); i++) {
        BlockType t = sprite.script[i].type;
        
        if (t == IF_START || t == IF_ELSE_START) {
            depth++;
        } else if (t == IF_END) {
            if (depth == 0) return i;
            depth--;
        } else if (t == ELSE_START && depth == 0) {
            return i;
        }
    }
    return sprite.script.size();
}

Variable* findVariable(Sprite &sprite, string name) {
    for (auto &var : sprite.variables) {
        if (var.name == name) {
            return &var;
        }
    }
    return nullptr;
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
            double oldX = sprite.x; 
            double oldY = sprite.y;
            double rad = toRadians(sprite.direction);
            
            sprite.x += current.val1 * cos(rad);
            sprite.y += current.val1 * sin(rad);
            
            if (sprite.isPenDown) {
                sprite.penTrail.push_back({oldX, oldY, sprite.x, sprite.y, sprite.penR, sprite.penG, sprite.penB, sprite.penThickness});
            }
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
            if (sprite.x > STAGE_WIDTH) {
                sprite.x = STAGE_WIDTH;
                sprite.direction = 180 - sprite.direction;
            } else if (sprite.x < 0) {
                sprite.x = 0;
                sprite.direction = 180 - sprite.direction;
            }
            if (sprite.y > STAGE_HEIGHT) {
                sprite.y = STAGE_HEIGHT;
                sprite.direction = 360 - sprite.direction;
            } else if (sprite.y < 0) {
                sprite.y = 0;
                sprite.direction = 360 - sprite.direction;
            }
            break;
        }

        case PEN_DOWN:
            sprite.isPenDown = true;
            break;
        case PEN_UP:
            sprite.isPenDown = false;
            break;
        case SET_PEN_COLOR:
            sprite.penR = (Uint8)current.val1;
            sprite.penG = (Uint8)current.val2;
            sprite.penB = (Uint8)current.val3;
            break;
        case CLEAR_ALL:
            sprite.penTrail.clear();
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
        case FOREVER_START: {
            LoopState newLoop;
            newLoop.startIndex = sprite.currentBlockIndex;
            newLoop.remaining = -1;
            newLoop.isForever = true;
            sprite.loopStack.push_back(newLoop);
            break;
        }
        case FOREVER_END: {
             if (!sprite.loopStack.empty()) {
                LoopState &top = sprite.loopStack.back();
                sprite.currentBlockIndex = top.startIndex + 1;
                autoAdvance = false;
             }
             break;
        }

        case IF_START: {
            bool isTrue = checkCondition(sprite, current.condType, current.condValue);
            if (!isTrue) {
                int jumpIndex = findMatchingEnd(sprite, sprite.currentBlockIndex);
                sprite.currentBlockIndex = jumpIndex; 
                autoAdvance = false; 
            }
            break;
        }
        case IF_ELSE_START: {
            bool isTrue = checkCondition(sprite, current.condType, current.condValue);
            if (!isTrue) {
                int jumpIndex = findMatchingEnd(sprite, sprite.currentBlockIndex);
                sprite.currentBlockIndex = jumpIndex;
                autoAdvance = false;
            }
            break;
        }
        case ELSE_START: {
            int jumpIndex = findMatchingEnd(sprite, sprite.currentBlockIndex);
            sprite.currentBlockIndex = jumpIndex;
            autoAdvance = false;
            break;
        }
        case IF_END:
            break;
        case WAIT_UNTIL: {
            bool isTrue = checkCondition(sprite, current.condType, current.condValue);
            if (!isTrue) {
                autoAdvance = false;
            }
            break;
        }

        case SET_VARIABLE_TO: {
            Variable* var = findVariable(sprite, current.strVal);
            if (var != nullptr) {
                var->value = current.val1;
                cout << "Variable [" << var->name << "] set to: " << var->value << endl;
            }
            break;
        }
        case CHANGE_VARIABLE_BY: {
            Variable* var = findVariable(sprite, current.strVal);
            if (var != nullptr) {
                var->value += current.val1;
                cout << "Variable [" << var->name << "] changed to: " << var->value << endl;
            }
            break;
        }
    }

    if (autoAdvance) {
        sprite.currentBlockIndex++;
    }
}

#endif