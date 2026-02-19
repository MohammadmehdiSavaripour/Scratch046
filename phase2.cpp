#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string>

struct Sprite {
    SDL_Texture* texture;
    int x, y;
    int w, h;
    int startX, startY;
};

Sprite loadSprite(SDL_Renderer* renderer, std::string path, int x, int y, int w, int h) {
    Sprite s;
    SDL_Surface* tempSurface = IMG_Load(path.c_str());
    s.texture = SDL_CreateTextureFromSurface(renderer, tempSurface);
    SDL_FreeSurface(tempSurface);

    s.x = x;
    s.y = y;
    s.w = w;
    s.h = h;
    s.startX = x;
    s.startY = y;

    return s;
}

void drawSprite(SDL_Renderer* renderer, Sprite &s) {
    SDL_Rect dest = { s.x, s.y, s.w, s.h };
    SDL_RenderCopy(renderer, s.texture, NULL, &dest);
}

void resetSprite(Sprite &s) {
    s.x = s.startX;
    s.y = s.startY;
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window* window = SDL_CreateWindow("Project Phase 2", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

    Sprite myPlayer = loadSprite(renderer, "assets/character.png", 100, 100, 100, 100);

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;

            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_RIGHT) myPlayer.x += 10;
                if (event.key.keysym.sym == SDLK_LEFT)  myPlayer.x -= 10;
                if (event.key.keysym.sym == SDLK_r)     resetSprite(myPlayer);
            }
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        drawSprite(renderer, myPlayer);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(myPlayer.texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}