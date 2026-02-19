#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <iostream>

#include "scratchengine.h"
#include "scratchdefinition.h"

using namespace std;

bool is_running = true;

void handle_events() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            is_running = false;
        }
        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                is_running = false;
            }
        }
    }
}

void update() {

}

void render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_RenderClear(renderer);

    SDL_Rect rect = {350, 250, 100, 100};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &rect);

    SDL_RenderPresent(renderer);
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        cout << "SDL init failed\n";
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
            "Scratch - Phase 1",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            800,
            600,
            SDL_WINDOW_SHOWN
    );

    if (!window) {
        cout << "Window creation failed\n";
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(
            window,
            -1,
            SDL_RENDERER_ACCELERATED
    );

    if (!renderer) {
        cout << "Renderer creation failed\n";
        return 1;
    }

    while (is_running) {
        handle_events();
        update();
        render(renderer);

        SDL_Delay(16); // ~60 FPS
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
