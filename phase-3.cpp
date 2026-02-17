#define SDL_MAIN_HANDLED
#include <iostream>
#include <vector>
#include <SDL2/SDL.h>
#include "scratchdefinition.h"
#include "scratchengine.h"

using namespace std;

void make_var(Sprite &s, string n, double v) {
    for (const auto& existing : s.variables) {
        if (existing.name == n) return;
    }
    s.variables.push_back({n, v});
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return 1;
    SDL_Window* win = SDL_CreateWindow("Phase 5", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, STAGE_WIDTH, STAGE_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    vector<Sprite> all_sprites;

    Sprite s1;
    s1.x = 200; s1.y = 300;
    s1.direction = 0; s1.isVisible = true; s1.size = 30;
    s1.currentBlockIndex = 0; s1.isRunning = true;
    s1.isPenDown = true;
    s1.penR = 255; s1.penG = 0; s1.penB = 0;
    s1.penThickness = 2;

    s1.script.push_back({FOREVER_START, 0, 0});
        s1.script.push_back({MOVE_STEPS, 3, 0});
        s1.script.push_back({IF_START, 0, 0, 0, "", KEY_SPACE_PRESSED, 0});
            s1.script.push_back({TURN_RIGHT, 5, 0});
        s1.script.push_back({IF_END, 0, 0});
        s1.script.push_back({IF_ON_EDGE_BOUNCE, 0, 0});
    s1.script.push_back({FOREVER_END, 0, 0});

    all_sprites.push_back(s1);

    Sprite s2;
    s2.x = 600; s2.y = 300;
    s2.direction = 180;
    s2.isVisible = true; s2.size = 20;
    s2.currentBlockIndex = 0; s2.isRunning = true;
    s2.isPenDown = true;
    s2.penR = 0; s2.penG = 0; s2.penB = 255;
    s2.penThickness = 4;

    s2.script.push_back({FOREVER_START, 0, 0});
        s2.script.push_back({MOVE_STEPS, 5, 0});
        s2.script.push_back({IF_ON_EDGE_BOUNCE, 0, 0});
        s2.script.push_back({IF_ELSE_START, 0, 0, 0, "", X_GREATER_THAN, 400});
             s2.script.push_back({TURN_RIGHT, 2, 0});
        s2.script.push_back({ELSE_START, 0, 0});
             s2.script.push_back({TURN_LEFT, 2, 0});
        s2.script.push_back({IF_END, 0, 0});
    s2.script.push_back({FOREVER_END, 0, 0});

    all_sprites.push_back(s2);

    bool done = false;
    SDL_Event event;

    while (!done) {
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) done = true;
        }
        
        SDL_PumpEvents();

        for (auto &s : all_sprites) {
            if (s.isRunning) {
                executeNextBlock(s);
            }
        }
        
        SDL_Delay(10); 

        SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
        SDL_RenderClear(ren);

        for (const auto &s : all_sprites) {
            for (const auto& l : s.penTrail) {
                SDL_SetRenderDrawColor(ren, l.r, l.g, l.b, 255);
                SDL_RenderDrawLine(ren, (int)l.x1, (int)l.y1, (int)l.x2, (int)l.y2);
            }
            
            if (s.isVisible) {
                SDL_Rect r = {(int)s.x, (int)s.y, (int)s.size, (int)s.size};
                SDL_SetRenderDrawColor(ren, s.penR, s.penG, s.penB, 255);
                SDL_RenderFillRect(ren, &r);
                
                int cx = (int)s.x + s.size/2;
                int cy = (int)s.y + s.size/2;
                SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
                SDL_RenderDrawLine(ren, cx, cy, cx + 20 * cos(s.direction * 3.1415/180.0), cy + 20 * sin(s.direction * 3.1415/180.0));
            }
        }

        SDL_RenderPresent(ren);
    }

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}