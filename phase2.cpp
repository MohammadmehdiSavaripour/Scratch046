#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <string>
#include <fstream>

using namespace std;

struct Sprite {
    SDL_Texture* texture;
    int x, y;
    int w, h;
    int startX, startY;
    bool isDragging;
};

void savePosition(Sprite &s) {
    ofstream outFile("save_pos.txt");
    if (outFile.is_open()) {
        outFile << s.x << " " << s.y;
        outFile.close();
        cout << "Position saved to file!" << endl;
    }
}

void loadPosition(Sprite &s) {
    ifstream inFile("save_pos.txt");
    if (inFile.is_open()) {
        inFile >> s.x >> s.y;
        inFile.close();
        cout << "Position loaded from file!" << endl;
    }
}

Sprite loadSprite(SDL_Renderer* renderer, string path, int x, int y, int w, int h) {
    Sprite s;
    SDL_Surface* tempSurface = IMG_Load(path.c_str());
    if (!tempSurface) cout << "Error loading image: " << IMG_GetError() << endl;
    s.texture = SDL_CreateTextureFromSurface(renderer, tempSurface);
    SDL_FreeSurface(tempSurface);

    s.x = x; s.y = y;
    s.w = w; s.h = h;
    s.startX = x; s.startY = y;
    s.isDragging = false;

    return s;
}

void drawSprite(SDL_Renderer* renderer, Sprite &s) {
    SDL_Rect dest = { s.x, s.y, s.w, s.h };
    SDL_RenderCopy(renderer, s.texture, NULL, &dest);
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_EVERYTHING);
    IMG_Init(IMG_INIT_PNG);

    SDL_Window* win = SDL_CreateWindow("Phase 2 & 5 Intro", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, 0);
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, 0);

    Sprite myPlayer = loadSprite(ren, "assets/character.png", 100, 100, 80, 80);

    bool running = true;
    SDL_Event e;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                if (e.button.button == SDL_BUTTON_LEFT) {
                    int mx = e.button.x;
                    int my = e.button.y;
                    if (mx >= myPlayer.x && mx <= myPlayer.x + myPlayer.w &&
                        my >= myPlayer.y && my <= myPlayer.y + myPlayer.h) {
                        myPlayer.isDragging = true;
                    }
                }
            }
            if (e.type == SDL_MOUSEBUTTONUP) {
                myPlayer.isDragging = false;
            }
            if (e.type == SDL_MOUSEMOTION) {
                if (myPlayer.isDragging) {
                    myPlayer.x = e.motion.x - (myPlayer.w / 2);
                    myPlayer.y = e.motion.y - (myPlayer.h / 2);
                }
            }

            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_s) savePosition(myPlayer);
                if (e.key.keysym.sym == SDLK_l) loadPosition(myPlayer);
                if (e.key.keysym.sym == SDLK_r) {
                    myPlayer.x = myPlayer.startX;
                    myPlayer.y = myPlayer.startY;
                }
            }
        }

        SDL_SetRenderDrawColor(ren, 240, 240, 240, 255);
        SDL_RenderClear(ren);

        drawSprite(ren, myPlayer);

        SDL_RenderPresent(ren);
    }

    SDL_DestroyTexture(myPlayer.texture);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    IMG_Quit();
    SDL_Quit();

    return 0;
}