#include <SDL2/SDL.h>
#include <iostream>

#include "scratchengine.h"
#include "scratchdefinition.h"

using namespace std;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;

bool is_running = false;

float sprite_x = 400.0f;
float sprite_y = 300.0f;
float sprite_speed = 200.0f;

bool move_up = false;
bool move_down = false;
bool move_left = false;
bool move_right = false;

bool init();
void handle_events();
void update(float delta_time);
void render();
void clean();

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    if (!init())
    {
        return -1;
    }

    const int target_fps = 60;
    const float target_frame_time = 1.0f / target_fps;

    float delta_time = 0.0f;
    Uint32 last_ticks = SDL_GetTicks();
    Uint32 frame_start = 0;

    is_running = true;

    while (is_running)
    {
        frame_start = SDL_GetTicks();

        Uint32 current_ticks = SDL_GetTicks();
        delta_time = (current_ticks - last_ticks) / 1000.0f;
        last_ticks = current_ticks;

        handle_events();
        update(delta_time);
        render();

        Uint32 frame_time = SDL_GetTicks() - frame_start;
        if (frame_time < (Uint32)(target_frame_time * 1000))
        {
            SDL_Delay((Uint32)(target_frame_time * 1000) - frame_time);
        }
    }

    clean();
    return 0;
}

bool init()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        return false;
    }

    window = SDL_CreateWindow(
            "Scratch - Phase 1",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            800,
            600,
            SDL_WINDOW_SHOWN
    );

    if (!window)
    {
        return false;
    }

    renderer = SDL_CreateRenderer(
            window,
            -1,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!renderer)
    {
        return false;
    }

    return true;
}

void handle_events()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            is_running = false;
        }
        else if (event.type == SDL_KEYDOWN)
        {
            if (event.key.keysym.sym == SDLK_ESCAPE)
                is_running = false;
            if (event.key.keysym.sym == SDLK_w)
                move_up = true;
            if (event.key.keysym.sym == SDLK_s)
                move_down = true;
            if (event.key.keysym.sym == SDLK_a)
                move_left = true;
            if (event.key.keysym.sym == SDLK_d)
                move_right = true;
        }
        else if (event.type == SDL_KEYUP)
        {
            if (event.key.keysym.sym == SDLK_w)
                move_up = false;
            if (event.key.keysym.sym == SDLK_s)
                move_down = false;
            if (event.key.keysym.sym == SDLK_a)
                move_left = false;
            if (event.key.keysym.sym == SDLK_d)
                move_right = false;
        }
    }
}

void update(float delta_time)
{
    if (move_up)
        sprite_y -= sprite_speed * delta_time;
    if (move_down)
        sprite_y += sprite_speed * delta_time;
    if (move_left)
        sprite_x -= sprite_speed * delta_time;
    if (move_right)
        sprite_x += sprite_speed * delta_time;
}

void render()
{
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);

    SDL_Rect sprite_rect;
    sprite_rect.x = (int)sprite_x;
    sprite_rect.y = (int)sprite_y;
    sprite_rect.w = 50;
    sprite_rect.h = 50;

    SDL_SetRenderDrawColor(renderer, 200, 80, 80, 255);
    SDL_RenderFillRect(renderer, &sprite_rect);

    SDL_RenderPresent(renderer);
}

void clean()
{
    if (renderer)
        SDL_DestroyRenderer(renderer);
    if (window)
        SDL_DestroyWindow(window);

    SDL_Quit();
}
