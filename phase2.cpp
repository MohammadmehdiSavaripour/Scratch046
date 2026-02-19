#include <SDL2/SDL.h>
#include <iostream>
#include <vector>

using namespace std;

struct Sprite {
    int x, y, w, h;
    int startX, startY;
};

struct UIBlock {
    int x, y, w, h;
    bool isSelected;
    SDL_Color color;
};

void resetSprite(Sprite &s) {
    s.x = s.startX;
    s.y = s.startY;
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window* win = SDL_CreateWindow("Phase 2 & 5", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, 0);
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, 0);

    Sprite cat = {400, 300, 50, 50, 400, 300};

    vector<UIBlock> blocks;
    blocks.push_back({50, 50, 100, 40, false, {255, 200, 0, 255}});
    blocks.push_back({50, 100, 100, 40, false, {0, 150, 255, 255}});

    bool running = true;
    SDL_Event e;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;

            if (e.type == SDL_MOUSEBUTTONDOWN) {
                for (auto &b : blocks) {
                    if (e.button.x >= b.x && e.button.x <= b.x + b.w &&
                        e.button.y >= b.y && e.button.y <= b.y + b.h) {
                        b.isSelected = true;
                    }
                }
            }
            if (e.type == SDL_MOUSEBUTTONUP) {
                for (auto &b : blocks) {
                    b.isSelected = false;
                }
            }
            if (e.type == SDL_MOUSEMOTION) {
                for (auto &b : blocks) {
                    if (b.isSelected) {
                        b.x = e.motion.x - (b.w / 2);
                        b.y = e.motion.y - (b.h / 2);
                    }
                }
            }

            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_r) resetSprite(cat);
                if (e.key.keysym.sym == SDLK_RIGHT) cat.x += 10;
            }
        }

        SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
        SDL_RenderClear(ren);

        SDL_Rect sideBar = {0, 0, 200, 600};
        SDL_SetRenderDrawColor(ren, 220, 220, 220, 255);
        SDL_RenderFillRect(ren, &sideBar);

        for (const auto &b : blocks) {
            SDL_Rect r = {b.x, b.y, b.w, b.h};
            SDL_SetRenderDrawColor(ren, b.color.r, b.color.g, b.color.b, 255);
            SDL_RenderFillRect(ren, &r);
        }
        SDL_Rect catRect = {cat.x, cat.y, cat.w, cat.h};
        SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);
        SDL_RenderFillRect(ren, &catRect);

        SDL_RenderPresent(ren);
    }

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}