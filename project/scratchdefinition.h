#ifndef SCRATCH_DEFINITION_H
#define SCRATCH_DEFINITION_H

#include <vector>
#include <string>
#include <SDL2/SDL.h>

using namespace std;

enum BlockType {
    MOVE_STEPS,
    TURN_RIGHT,
    TURN_LEFT,
    GO_TO_XY,
    IF_ON_EDGE_BOUNCE,
    PEN_DOWN,
    PEN_UP,
    SET_PEN_COLOR,
    CLEAR_ALL,
    REPEAT_START,
    REPEAT_END,
    FOREVER_START,
    FOREVER_END,
    IF_START,
    IF_ELSE_START,
    ELSE_START,
    IF_END,
    SET_VARIABLE_TO,
    CHANGE_VARIABLE_BY,
    WAIT_SECONDS
};

enum ConditionType {
    KEY_SPACE_PRESSED,
    MOUSE_CLICKED,
    TOUCHING_EDGE,
    X_GREATER_THAN,
    X_LESS_THAN
};

struct Block {
    BlockType type;
    double val1;
    double val2;
    double val3;
    string strVal;
    ConditionType condType;
    double condValue;
};

struct Line {
    double x1, y1, x2, y2;
    Uint8 r, g, b;
    int thickness;
};

struct LoopState {
    int startIndex;    
    int remaining;     
    bool isForever;    
};

struct Variable {
    string name;
    double value;
};

struct Sprite {
    double x, y;
    double direction;
    bool isVisible;
    double size;

    bool isPenDown;
    Uint8 penR, penG, penB;
    int penThickness;
    vector<Line> penTrail;

    vector<Block> script;
    int currentBlockIndex;
    bool isRunning;
    
    vector<LoopState> loopStack;
    vector<Variable> variables;

    Uint32 wait_timer;
};

#endif