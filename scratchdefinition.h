#ifndef SCRATCH_TYPES_H
#define SCRATCH_TYPES_H

#include <vector>
#include <string>
enum BlockType {
    MOVE_STEPS,
    TURN_RIGHT,
    TURN_LEFT,
    GO_TO_XY,
    IF_ON_EDGE_BOUNCE 
};

struct Block {
    BlockType type;
    double val1;
    double val2;
};

struct Sprite {
    double x, y;
    double direction;
    bool isVisible;
    double size;

    std::vector<Block> script;
    int currentBlockIndex;
    bool isRunning;
};
#endif