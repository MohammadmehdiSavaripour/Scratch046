#include <SDL2/SDL.h>
#include <iostream>

#include "scratchengine.h"
#include "scratchdefinition.h"

using namespace std;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;

bool is_running = false;

const int window_width = 800;
const int window_height = 600;

float sprite_x = 400.0f;
float sprite_y = 300.0f;
float sprite_speed = 200.0f;
const int sprite_size = 50;

bool move_up = false;
bool move_down = false;
bool move_left = false;
bool move_right = false;

int mouse_x = 0;
int mouse_y = 0;
bool is_mouse_down = false;

Uint32 start_ticks = 0;
float total_timer = 0.0f;

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
        return -1;

    start_ticks = SDL_GetTicks();
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

        total_timer = (current_ticks - start_ticks) / 1000.0f;

        handle_events();
        update(delta_time);
        render();

        Uint32 frame_time = SDL_GetTicks() - frame_start;
        if (frame_time < (Uint32)(target_frame_time * 1000))
            SDL_Delay((Uint32)(target_frame_time * 1000) - frame_time);
    }

    clean();
    return 0;
}

bool init()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        return false;

    window = SDL_CreateWindow(
            "Scratch - Phase 1 Complete",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            window_width,
            window_height,
            SDL_WINDOW_SHOWN
    );

    if (!window)
        return false;

    renderer = SDL_CreateRenderer(
            window,
            -1,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!renderer)
        return false;

    return true;
}

void handle_events()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
            is_running = false;

        if (event.type == SDL_MOUSEMOTION)
        {
            SDL_GetMouseState(&mouse_x, &mouse_y);
        }

        if (event.type == SDL_MOUSEBUTTONDOWN)
        {
            if (event.button.button == SDL_BUTTON_LEFT)
                is_mouse_down = true;
        }
        else if (event.type == SDL_MOUSEBUTTONUP)
        {
            if (event.button.button == SDL_BUTTON_LEFT)
                is_mouse_down = false;
        }

        if (event.type == SDL_KEYDOWN)
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
            if (event.key.keysym.sym == SDLK_r)
            {
                sprite_x = 400.0f;
                sprite_y = 300.0f;
            }
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

    if (sprite_x < 0)
        sprite_x = 0;
    if (sprite_y < 0)
        sprite_y = 0;
    if (sprite_x > window_width - sprite_size)
        sprite_x = window_width - sprite_size;
    if (sprite_y > window_height - sprite_size)
        sprite_y = window_height - sprite_size;
}

void render()
{
    if (is_mouse_down)
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    else
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);

    SDL_RenderClear(renderer);

    SDL_Rect sprite_rect;
    sprite_rect.x = (int)sprite_x;
    sprite_rect.y = (int)sprite_y;
    sprite_rect.w = sprite_size;
    sprite_rect.h = sprite_size;

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
