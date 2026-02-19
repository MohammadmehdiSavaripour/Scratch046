#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <iostream>
#include <vector>

using namespace std;

struct Button {
    int x, y, w, h;
    SDL_Color color;
};

struct UIBlock {
    int x, y, w, h;
    bool isSelected;
    SDL_Color color;
};

void drawRect(SDL_Renderer* ren, int x, int y, int w, int h, SDL_Color c) {
    SDL_Rect r = {x, y, w, h};
    SDL_SetRenderDrawColor(ren, c.r, c.g, c.b, c.a);
    SDL_RenderFillRect(ren, &r);
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    SDL_RenderDrawRect(ren, &r);
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return 1;
    SDL_Window* win = SDL_CreateWindow("Phase 5: Menu & Play/Stop", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, 0);
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    Button btnPlay = {650, 20, 60, 40, {0, 200, 0, 255}};
    Button btnStop = {720, 20, 60, 40, {200, 0, 0, 255}};

    vector<UIBlock> blocks;
    blocks.push_back({20, 80, 160, 40, false, {0, 150, 255, 255}});
    blocks.push_back({20, 140, 160, 40, false, {255, 165, 0, 255}});

    bool running = true;
    bool isPlaying = false;
    SDL_Event e;

    while (running) {
        int mx, my;
        SDL_GetMouseState(&mx, &my);

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;

            if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                if (mx >= btnPlay.x && mx <= btnPlay.x + btnPlay.w && my >= btnPlay.y && my <= btnPlay.y + btnPlay.h) {
                    isPlaying = true;
                    cout << "Engine Status: PLAYING\n";
                }
                if (mx >= btnStop.x && mx <= btnStop.x + btnStop.w && my >= btnStop.y && my <= btnStop.y + btnStop.h) {
                    isPlaying = false;
                    cout << "Engine Status: STOPPED\n";
                }
                for (auto& b : blocks) {
                    if (mx >= b.x && mx <= b.x + b.w && my >= b.y && my <= b.y + b.h) {
                        b.isSelected = true;
                    }
                }
            }

            if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
                for (auto& b : blocks) b.isSelected = false;
            }

            if (e.type == SDL_MOUSEMOTION) {
                for (auto& b : blocks) {
                    if (b.isSelected) {
                        b.x += e.motion.xrel;
                        b.y += e.motion.yrel;
                    }
                }
            }
        }

        SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
        SDL_RenderClear(ren);

        drawRect(ren, 0, 0, 200, 600, {230, 230, 230, 255});

        SDL_Color playColor = isPlaying ? SDL_Color{0, 255, 0, 255} : btnPlay.color;
        SDL_Color stopColor = !isPlaying ? SDL_Color{255, 50, 50, 255} : btnStop.color;
        drawRect(ren, btnPlay.x, btnPlay.y, btnPlay.w, btnPlay.h, playColor);
        drawRect(ren, btnStop.x, btnStop.y, btnStop.w, btnStop.h, stopColor);

        for (const auto& b : blocks) {
            drawRect(ren, b.x, b.y, b.w, b.h, b.color);
        }

        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}