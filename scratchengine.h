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

bool checkCondition(Sprite &s, ConditionType type, double value) {
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
            return (s.x <= 0 || s.x >= STAGE_WIDTH ||
                    s.y <= 0 || s.y >= STAGE_HEIGHT);
        }
        case X_GREATER_THAN:
            return s.x > value;
        case X_LESS_THAN:
            return s.x < value;
    }
    return false;
}

int findMatchingEnd(Sprite &s, int startIndex) {
    int depth = 0;
    for (int i = startIndex + 1; i < s.script.size(); i++) {
        BlockType t = s.script[i].type;

        if (t == IF_START || t == IF_ELSE_START) {
            depth++;
        } else if (t == IF_END) {
            if (depth == 0) return i;
            depth--;
        } else if (t == ELSE_START && depth == 0) {
            return i;
        }
    }
    return s.script.size();
}

Variable* findVariable(Sprite &s, string name) {
    for (auto &var : s.variables) {
        if (var.name == name) {
            return &var;
        }
    }
    return nullptr;
}

void executeNextBlock(Sprite &s) {
    if (!s.isRunning || s.currentBlockIndex >= s.script.size()) {
        s.isRunning = false;
        return;
    }

    Block &current = s.script[s.currentBlockIndex];
    bool autoAdvance = true;

    switch (current.type) {
        case MOVE_STEPS: {
            double oldX = s.x;
            double oldY = s.y;
            double rad = toRadians(s.direction);

            s.x += current.val1 * cos(rad);
            s.y += current.val1 * sin(rad);

            if (s.isPenDown) {
                s.penTrail.push_back({oldX, oldY, s.x, s.y, s.penR, s.penG, s.penB, s.penThickness});
            }
            break;
        }
        case TURN_RIGHT:
            s.direction += current.val1;
            break;
        case TURN_LEFT:
            s.direction -= current.val1;
            break;
        case GO_TO_XY:
            s.x = current.val1;
            s.y = current.val2;
            break;
        case IF_ON_EDGE_BOUNCE: {
            if (s.x > STAGE_WIDTH) {
                s.x = STAGE_WIDTH;
                s.direction = 180 - s.direction;
            } else if (s.x < 0) {
                s.x = 0;
                s.direction = 180 - s.direction;
            }
            if (s.y > STAGE_HEIGHT) {
                s.y = STAGE_HEIGHT;
                s.direction = 360 - s.direction;
            } else if (s.y < 0) {
                s.y = 0;
                s.direction = 360 - s.direction;
            }
            break;
        }
        case PEN_DOWN:
            s.isPenDown = true;
            break;
        case PEN_UP:
            s.isPenDown = false;
            break;
        case SET_PEN_COLOR:
            s.penR = (Uint8)current.val1;
            s.penG = (Uint8)current.val2;
            s.penB = (Uint8)current.val3;
            break;
        case CLEAR_ALL:
            s.penTrail.clear();
            break;
        case REPEAT_START: {
            LoopState newLoop;
            newLoop.startIndex = s.currentBlockIndex;
            newLoop.remaining = (int)current.val1;
            newLoop.isForever = false;
            s.loopStack.push_back(newLoop);
            break;
        }
        case REPEAT_END: {
            if (!s.loopStack.empty()) {
                LoopState &top = s.loopStack.back();
                if (!top.isForever) {
                    top.remaining--;
                }

                if (top.isForever || top.remaining > 0) {
                    s.currentBlockIndex = top.startIndex + 1;
                    autoAdvance = false;
                } else {
                    s.loopStack.pop_back();
                }
            }
            break;
        }
        case FOREVER_START: {
            LoopState newLoop;
            newLoop.startIndex = s.currentBlockIndex;
            newLoop.remaining = -1;
            newLoop.isForever = true;
            s.loopStack.push_back(newLoop);
            break;
        }
        case FOREVER_END: {
             if (!s.loopStack.empty()) {
                LoopState &top = s.loopStack.back();
                s.currentBlockIndex = top.startIndex + 1;
                autoAdvance = false;
             }
             break;
        }
        case IF_START: {
            bool isTrue = checkCondition(s, current.condType, current.condValue);
            if (!isTrue) {
                int jumpIndex = findMatchingEnd(s, s.currentBlockIndex);
                s.currentBlockIndex = jumpIndex;
                autoAdvance = false;
            }
            break;
        }
        case IF_ELSE_START: {
            bool isTrue = checkCondition(s, current.condType, current.condValue);
            if (!isTrue) {
                int jumpIndex = findMatchingEnd(s, s.currentBlockIndex);
                s.currentBlockIndex = jumpIndex;
                autoAdvance = false;
            }
            break;
        }
        case ELSE_START: {
            int jumpIndex = findMatchingEnd(s, s.currentBlockIndex);
            s.currentBlockIndex = jumpIndex;
            autoAdvance = false;
            break;
        }
        case IF_END:
            break;
        case SET_VARIABLE_TO: {
            Variable* var = findVariable(s, current.strVal);
            if (var != nullptr) {
                var->value = current.val1;
            }
            break;
        }
        case CHANGE_VARIABLE_BY: {
            Variable* var = findVariable(s, current.strVal);
            if (var != nullptr) {
                var->value += current.val1;
            }
            break;
        }
        case WAIT_SECONDS: {
            Uint32 now = SDL_GetTicks();
            if (s.wait_timer == 0) {
                s.wait_timer = now;
                autoAdvance = false;
            } else {
                if (now - s.wait_timer < current.val1 * 1000) {
                    autoAdvance = false;
                } else {
                    s.wait_timer = 0;
                }
            }
            break;
        }
    }

    if (autoAdvance) {
        s.currentBlockIndex++;
    }
}

#endif