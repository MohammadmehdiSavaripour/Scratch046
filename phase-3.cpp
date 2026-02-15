#define SDL_MAIN_HANDLED
#include <iostream>
#include <SDL2/SDL.h>
#include "scratchdefinition.h"
#include "scratchengine.h"

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    
    SDL_Window* window = SDL_CreateWindow("Scratch Phase 3 - Step 1", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, STAGE_WIDTH, STAGE_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    Sprite cat;
    cat.x = 100;
    cat.y = 100;
    cat.direction = 0;
    cat.isVisible = true;
    cat.size = 50;
    cat.currentBlockIndex = 0;
    cat.isRunning = true;

    for(int i=0; i<50; i++) {
        cat.script.push_back({MOVE_STEPS, 5, 0});
    }
    cat.script.push_back({TURN_RIGHT, 90, 0});
    for(int i=0; i<50; i++) {
        cat.script.push_back({MOVE_STEPS, 5, 0});
    }

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

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        SDL_Rect rect = {(int)cat.x, (int)cat.y, (int)cat.size, (int)cat.size};
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &rect);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}