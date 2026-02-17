#define SDL_MAIN_HANDLED
#include <iostream>
#include <SDL2/SDL.h>
#include "scratchdefinition.h"
#include "scratchengine.h"
using namespace std;

void addVariable(Sprite &sprite, string name, double initialValue) {
    for (const auto& v : sprite.variables) {
        if (v.name == name) return;
    }
    sprite.variables.push_back({name, initialValue});
    cout << "Variable created: " << name << endl;
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        cout << "SDL Error: " << SDL_GetError() << endl;
        return 1;
    }
    
    SDL_Window* window = SDL_CreateWindow("Phase 3 & 4: Final Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, STAGE_WIDTH, STAGE_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    Sprite cat;
    cat.x = 400; cat.y = 300;
    cat.direction = 0;
    cat.isVisible = true; cat.size = 20;
    cat.currentBlockIndex = 0; cat.isRunning = true;
    
    cat.isPenDown = true;
    cat.penR = 0; cat.penG = 100; cat.penB = 255; 
    cat.penThickness = 2;

    addVariable(cat, "Score", 0);
    
    cat.script.push_back({FOREVER_START, 0, 0});
        
        cat.script.push_back({MOVE_STEPS, 4, 0});

        cat.script.push_back({IF_ELSE_START, 0, 0, 0, "", KEY_SPACE_PRESSED, 0});
            cat.script.push_back({TURN_RIGHT, 5, 0});
            
            Block changeScore;
            changeScore.type = CHANGE_VARIABLE_BY;
            changeScore.val1 = 1;
            changeScore.strVal = "Score";
            cat.script.push_back(changeScore);
            
            cat.script.push_back({SET_PEN_COLOR, 255, 0, 0});

        cat.script.push_back({ELSE_START, 0, 0});
            cat.script.push_back({SET_PEN_COLOR, 0, 100, 255});
        cat.script.push_back({IF_END, 0, 0});

        cat.script.push_back({IF_ON_EDGE_BOUNCE, 0, 0});

    cat.script.push_back({FOREVER_END, 0, 0});

    bool quit = false;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) quit = true;
        }
        
        SDL_PumpEvents();

        if (cat.isRunning) {
            executeNextBlock(cat);
            SDL_Delay(10); 
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        for (const auto& line : cat.penTrail) {
            SDL_SetRenderDrawColor(renderer, line.r, line.g, line.b, 255);
            SDL_RenderDrawLine(renderer, (int)line.x1, (int)line.y1, (int)line.x2, (int)line.y2);
        }

        if (cat.isVisible) {
            SDL_Rect rect = {(int)cat.x, (int)cat.y, (int)cat.size, (int)cat.size};
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderFillRect(renderer, &rect);
        
            int cx = (int)cat.x + cat.size/2;
            int cy = (int)cat.y + cat.size/2;
            SDL_RenderDrawLine(renderer, cx, cy, cx + 20 * cos(cat.direction * PI/180.0), cy + 20 * sin(cat.direction * PI/180.0));
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}