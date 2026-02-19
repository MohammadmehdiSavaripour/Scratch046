#include <SDL2/SDL.h>
#include <iostream>
#include <string>
#include <cmath>
#include <cstdlib>
#include <ctime>

using namespace std;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
bool is_running = false;

const int WINDOW_WIDTH  = 800;
const int WINDOW_HEIGHT = 600;

float sprite_x = 400.0f;
float sprite_y = 300.0f;
const int SPRITE_SIZE = 50;

float sprite2_x = 200.0f;
float sprite2_y = 200.0f;
const int SPRITE2_SIZE = 50;

int mouse_x = 0;
int mouse_y = 0;
bool is_mouse_down = false;

bool is_dragging = false;
int drag_offset_x = 0;
int drag_offset_y = 0;

Uint32 start_ticks = 0;
float timer_seconds = 0.0f;

bool key_states[SDL_NUM_SCANCODES] = {false};

bool asking_question = false;
string question_text = "";
string answer_text   = "";
bool answer_ready    = false;

bool touching_sprite2   = false;
bool touching_edge      = false;
float distance_to_mouse = 0.0f;
float distance_to_sprite2 = 0.0f;

bool sensing_is_touching_sprite(
        float ax, float ay, int aw, int ah,
        float bx, float by, int bw, int bh)
{
    return (ax < bx + bw) && (ax + aw > bx) &&
           (ay < by + bh) && (ay + ah > by);
}

bool sensing_is_touching_edge(float sx, float sy, int sw, int sh)
{
    return (sx <= 0) || (sy <= 0) ||
           (sx + sw >= WINDOW_WIDTH) || (sy + sh >= WINDOW_HEIGHT);
}

float sensing_distance(float x1, float y1, float x2, float y2)
{
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrt(dx * dx + dy * dy);
}

bool sensing_key_pressed(SDL_Scancode key)
{
    return key_states[key];
}

bool sensing_mouse_down()
{
    return is_mouse_down;
}

int sensing_mouse_x() { return mouse_x; }
int sensing_mouse_y() { return mouse_y; }

float sensing_timer()
{
    return timer_seconds;
}

void sensing_reset_timer()
{
    start_ticks = SDL_GetTicks();
}

void sensing_start_drag(int mx, int my)
{
    if (mx >= (int)sprite_x && mx <= (int)sprite_x + SPRITE_SIZE &&
        my >= (int)sprite_y && my <= (int)sprite_y + SPRITE_SIZE)
    {
        is_dragging = true;
        drag_offset_x = mx - (int)sprite_x;
        drag_offset_y = my - (int)sprite_y;
    }
}

void sensing_update_drag(int mx, int my)
{
    if (is_dragging)
    {
        sprite_x = (float)(mx - drag_offset_x);
        sprite_y = (float)(my - drag_offset_y);

        if (sprite_x < 0) sprite_x = 0;
        if (sprite_y < 0) sprite_y = 0;
        if (sprite_x + SPRITE_SIZE > WINDOW_WIDTH)
            sprite_x = (float)(WINDOW_WIDTH - SPRITE_SIZE);
        if (sprite_y + SPRITE_SIZE > WINDOW_HEIGHT)
            sprite_y = (float)(WINDOW_HEIGHT - SPRITE_SIZE);
    }
}

void sensing_stop_drag()
{
    is_dragging = false;
}

void sensing_ask(const string& question)
{
    asking_question = true;
    question_text   = question;
    answer_text     = "";
    answer_ready    = false;
    cout << "\n[ASK] " << question << endl;
    cout << "[ANS] ";
}

bool init()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        return false;

    window = SDL_CreateWindow(
            "Scratch - Phase 6 Part 1 (Sensing)",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            WINDOW_WIDTH,
            WINDOW_HEIGHT,
            SDL_WINDOW_SHOWN
    );
    if (!window) return false;

    renderer = SDL_CreateRenderer(window, -1,
                                  SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) return false;

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
            mouse_x = event.motion.x;
            mouse_y = event.motion.y;
            sensing_update_drag(mouse_x, mouse_y);
        }

        if (event.type == SDL_MOUSEBUTTONDOWN)
        {
            if (event.button.button == SDL_BUTTON_LEFT)
            {
                is_mouse_down = true;
                sensing_start_drag(event.button.x, event.button.y);
            }
        }

        if (event.type == SDL_MOUSEBUTTONUP)
        {
            if (event.button.button == SDL_BUTTON_LEFT)
            {
                is_mouse_down = false;
                sensing_stop_drag();
            }
        }

        if (event.type == SDL_KEYDOWN)
        {
            key_states[event.key.keysym.scancode] = true;

            if (event.key.keysym.sym == SDLK_ESCAPE)
                is_running = false;

            if (event.key.keysym.sym == SDLK_r)
            {
                sensing_reset_timer();
                cout << "[TIMER] Timer reset!\n";
            }

            if (event.key.keysym.sym == SDLK_q)
            {
                sensing_ask("What is your name?");
            }
        }

        if (event.type == SDL_KEYUP)
        {
            key_states[event.key.keysym.scancode] = false;
        }

        if (asking_question && event.type == SDL_TEXTINPUT)
        {
            answer_text += event.text.text;
        }

        if (asking_question && event.type == SDL_KEYDOWN)
        {
            if (event.key.keysym.sym == SDLK_RETURN)
            {
                asking_question = false;
                answer_ready    = true;
                cout << answer_text << endl;
                cout << "[ANSWER STORED]: " << answer_text << endl;
            }
            else if (event.key.keysym.sym == SDLK_BACKSPACE && !answer_text.empty())
            {
                answer_text.pop_back();
            }
        }
    }
}

void update()
{
    timer_seconds = (SDL_GetTicks() - start_ticks) / 1000.0f;

    touching_sprite2 = sensing_is_touching_sprite(
            sprite_x, sprite_y, SPRITE_SIZE, SPRITE_SIZE,
            sprite2_x, sprite2_y, SPRITE2_SIZE, SPRITE2_SIZE
    );

    touching_edge = sensing_is_touching_edge(
            sprite_x, sprite_y, SPRITE_SIZE, SPRITE_SIZE
    );

    distance_to_mouse = sensing_distance(
            sprite_x + SPRITE_SIZE / 2.0f,
            sprite_y + SPRITE_SIZE / 2.0f,
            (float)mouse_x,
            (float)mouse_y
    );

    distance_to_sprite2 = sensing_distance(
            sprite_x + SPRITE_SIZE / 2.0f,
            sprite_y + SPRITE_SIZE / 2.0f,
            sprite2_x + SPRITE2_SIZE / 2.0f,
            sprite2_y + SPRITE2_SIZE / 2.0f
    );

    static float last_print = 0.0f;
    if (timer_seconds - last_print >= 2.0f)
    {
        last_print = timer_seconds;
        cout << "====== SENSING INFO ======\n";
        cout << "Timer: " << timer_seconds << "s\n";
        cout << "Touching Sprite2: " << (touching_sprite2 ? "YES" : "NO") << "\n";
        cout << "Touching Edge: "    << (touching_edge    ? "YES" : "NO") << "\n";
        cout << "Distance to Mouse: "<< (int)distance_to_mouse << "px\n";
        cout << "Distance to Sprite2: " << (int)distance_to_sprite2 << "px\n";
        cout << "Mouse Down: " << (sensing_mouse_down() ? "YES" : "NO") << "\n";
        cout << "Space Key: " << (sensing_key_pressed(SDL_SCANCODE_SPACE) ? "YES" : "NO") << "\n";
        if (answer_ready)
            cout << "Last Answer: " << answer_text << "\n";
        cout << "==========================\n";
    }
}

void draw_info_box(int x, int y, int w, int h, SDL_Color color, bool active)
{
    if (active)
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
    else
        SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);

    SDL_Rect r = {x, y, w, h};
    SDL_RenderFillRect(renderer, &r);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &r);
}

void render()
{
    SDL_SetRenderDrawColor(renderer, 25, 25, 40, 255);
    SDL_RenderClear(renderer);

    if (touching_sprite2)
        SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255);
    else
        SDL_SetRenderDrawColor(renderer, 150, 50, 50, 255);

    SDL_Rect rect2 = {(int)sprite2_x, (int)sprite2_y, SPRITE2_SIZE, SPRITE2_SIZE};
    SDL_RenderFillRect(renderer, &rect2);
    SDL_SetRenderDrawColor(renderer, 200, 100, 100, 255);
    SDL_RenderDrawRect(renderer, &rect2);

    if (is_dragging)
        SDL_SetRenderDrawColor(renderer, 100, 200, 255, 255);
    else if (touching_sprite2)
        SDL_SetRenderDrawColor(renderer, 255, 200, 50, 255);
    else if (touching_edge)
        SDL_SetRenderDrawColor(renderer, 255, 80, 80, 255);
    else
        SDL_SetRenderDrawColor(renderer, 80, 160, 220, 255);

    SDL_Rect rect1 = {(int)sprite_x, (int)sprite_y, SPRITE_SIZE, SPRITE_SIZE};
    SDL_RenderFillRect(renderer, &rect1);
    SDL_SetRenderDrawColor(renderer, 150, 220, 255, 255);
    SDL_RenderDrawRect(renderer, &rect1);

    SDL_SetRenderDrawColor(renderer, 100, 255, 100, 100);
    SDL_RenderDrawLine(renderer,
                       (int)(sprite_x + SPRITE_SIZE / 2),
                       (int)(sprite_y + SPRITE_SIZE / 2),
                       mouse_x, mouse_y);

    SDL_Color green  = {50, 200, 50, 255};
    SDL_Color orange = {220, 150, 30, 255};
    SDL_Color cyan   = {50, 200, 200, 255};
    SDL_Color pink   = {220, 100, 180, 255};
    SDL_Color blue   = {80, 120, 220, 255};

    draw_info_box(10,  530, 140, 25, green,  touching_sprite2);
    draw_info_box(160, 530, 140, 25, orange, touching_edge);
    draw_info_box(310, 530, 140, 25, cyan,   is_mouse_down);
    draw_info_box(460, 530, 140, 25, pink,   is_dragging);
    draw_info_box(610, 530, 140, 25, blue,   sensing_key_pressed(SDL_SCANCODE_SPACE));

    int timer_bar_w = (int)(timer_seconds * 20) % WINDOW_WIDTH;
    SDL_SetRenderDrawColor(renderer, 80, 80, 200, 255);
    SDL_Rect timer_bar = {0, 560, timer_bar_w, 10};
    SDL_RenderFillRect(renderer, &timer_bar);

    if (asking_question)
    {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
        SDL_Rect qbox = {100, 220, 600, 120};
        SDL_RenderFillRect(renderer, &qbox);

        SDL_SetRenderDrawColor(renderer, 100, 200, 255, 255);
        SDL_RenderDrawRect(renderer, &qbox);

        SDL_SetRenderDrawColor(renderer, 30, 30, 50, 255);
        SDL_Rect ans_box = {110, 290, 580, 40};
        SDL_RenderFillRect(renderer, &ans_box);
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_RenderDrawRect(renderer, &ans_box);
    }

    SDL_RenderPresent(renderer);
}

void clean()
{
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window)   SDL_DestroyWindow(window);
    SDL_Quit();
}

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    srand((unsigned int)time(nullptr));

    if (!init())
    {
        cout << "SDL Init failed: " << SDL_GetError() << endl;
        return -1;
    }

    start_ticks = SDL_GetTicks();
    is_running  = true;

    Uint32 last_ticks = SDL_GetTicks();
    const float TARGET_FPS = 60.0f;
    const float FRAME_TIME = 1.0f / TARGET_FPS;

    cout << "=== Phase 6 Part 1: Sensing ===\n";
    cout << "Controls:\n";
    cout << "  Mouse Click + Drag = Drag sprite\n";
    cout << "  R = Reset timer\n";
    cout << "  Q = Ask question\n";
    cout << "  Space = Test key sensing\n";
    cout << "  ESC = Quit\n";
    cout << "Indicators (bottom bar):\n";
    cout << "  [Green]  = Touching Sprite2\n";
    cout << "  [Orange] = Touching Edge\n";
    cout << "  [Cyan]   = Mouse Down\n";
    cout << "  [Pink]   = Dragging\n";
    cout << "  [Blue]   = Space Pressed\n";
    cout << "  [Bottom bar] = Timer progress\n\n";

    SDL_StartTextInput();

    while (is_running)
    {
        Uint32 frame_start = SDL_GetTicks();

        Uint32 current = SDL_GetTicks();
        (void)(current - last_ticks);
        last_ticks = current;

        handle_events();
        update();
        render();

        Uint32 elapsed = SDL_GetTicks() - frame_start;
        Uint32 delay   = (Uint32)(FRAME_TIME * 1000);
        if (elapsed < delay)
            SDL_Delay(delay - elapsed);
    }

    SDL_StopTextInput();
    clean();
    return 0;
}
