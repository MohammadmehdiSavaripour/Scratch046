#define SDL_MAIN_HANDLED
#include <iostream>
#include <SDL2/SDL.h>
#include "scratchdefinition.h"
#include "scratchengine.h"

using namespace std;

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        cout << "SDL Error: " << SDL_GetError() << endl;
        return 1;
    }
    
    SDL_Window* window = SDL_CreateWindow("Phase 3 - Step 3: Loops", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, STAGE_WIDTH, STAGE_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    Sprite cat;
    cat.x = 400; cat.y = 300;
    cat.direction = 0;
    cat.isVisible = true; cat.size = 20;
    cat.currentBlockIndex = 0; cat.isRunning = true;
    
    cat.isPenDown = true;
    cat.penR = 255; cat.penG = 0; cat.penB = 100; cat.penThickness = 2;

    cat.script.push_back({REPEAT_START, 12, 0}); 
        
        cat.script.push_back({REPEAT_START, 4, 0});
            cat.script.push_back({MOVE_STEPS, 80, 0});
            cat.script.push_back({TURN_RIGHT, 90, 0});
        cat.script.push_back({REPEAT_END, 0, 0});
        cat.script.push_back({TURN_RIGHT, 30, 0});
        
    cat.script.push_back({REPEAT_END, 0, 0});

    bool quit = false;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) quit = true;
        }

        if (cat.isRunning) {
            executeNextBlock(cat);
            SDL_Delay(20);
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // زمینه سفید
        SDL_RenderClear(renderer);

        for (const auto& line : cat.penTrail) {
            SDL_SetRenderDrawColor(renderer, line.r, line.g, line.b, 255);
            SDL_RenderDrawLine(renderer, (int)line.x1, (int)line.y1, (int)line.x2, (int)line.y2);
        }
        if (cat.isVisible) {
            SDL_Rect rect = {(int)cat.x, (int)cat.y, (int)cat.size, (int)cat.size};
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderFillRect(renderer, &rect);
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}