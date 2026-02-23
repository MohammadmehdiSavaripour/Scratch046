#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <map>
#include <chrono>

using namespace std;
using namespace std::chrono;

const int PANEL_HEIGHT  = 130;
const int TOOLBAR_H     = 50;
const int PALETTE_WIDTH = 220;


const int BLOCK_H       = 32;
const int BLOCK_W       = 200;


const int TAB_W  = 20;
const int TAB_H  = 7;
const int TAB_OX = 18;


struct TextureData { SDL_Texture* tex; int w, h; };

struct SayBubble {
    string text;
    bool active = false;
    bool isThink = false;
    float timeLeft = -1.f;
};


struct Sprite {
    string name;
    float x, y, startX, startY;
    float direction, startDirection;
    float sizePct, startSizePct;
    bool visible;
    int layer;
    SayBubble bubble;
    vector<TextureData> costumes;
    int currentCostume;
    bool beingDragged;
    int dragOffX, dragOffY;


    bool      isPenDown = false;
    SDL_Color penColor  = {200, 0, 0, 255};
    float     penSize   = 2.0f;
};


enum CondType {
    COND_VAR_CMP,
    COND_X_CMP,
    COND_Y_CMP,
    COND_DIR_CMP,
    COND_SIZE_CMP,
    COND_TIMER_CMP,
    COND_TOUCHING_EDGE,
    COND_ALWAYS_TRUE,
    COND_ALWAYS_FALSE,
    COND_KEY_LEFT,
    COND_KEY_RIGHT,
    COND_KEY_UP,
    COND_KEY_DOWN,
    COND_KEY_SPACE
};

struct Condition {
    CondType type = COND_VAR_CMP;
    string varName;
    string op;
    double rhs = 0.0;
};

enum BlockType {
    CMD_MOVE, CMD_TURN_RIGHT, CMD_TURN_LEFT, CMD_GOTO,
    CMD_CHANGE_X, CMD_CHANGE_Y, CMD_SET_X, CMD_SET_Y,
    CMD_POINT_DIR, CMD_POINT_TOWARDS_CENTER,
    CMD_SAY, CMD_THINK,
    CMD_SHOW, CMD_HIDE, CMD_SET_SIZE, CMD_CHANGE_SIZE, CMD_NEXT_COSTUME,
    CMD_WAIT, CMD_REPEAT, CMD_END_REPEAT, CMD_FOREVER,
    CMD_IF, CMD_ELSE, CMD_END_IF,
    CMD_REPEAT_UNTIL, CMD_END_REPEAT_UNTIL,
    CMD_STOP_ALL,
    CMD_SET_VAR, CMD_CHANGE_VAR,
    CMD_RESET_TIMER, CMD_RESET,
    CMD_BOUNCE_IF_EDGE,
    CMD_FOREVER_BOUNCE, CMD_END_FOREVER_BOUNCE,
    CMD_GLIDE,
    CMD_WAIT_UNTIL,
    CMD_GO_TO_RANDOM,
    CMD_SET_VAR_RANDOM,
    CMD_REPEAT_FOR_SECONDS,
    CMD_END_REPEAT_FOR_SECONDS,


    CMD_BROADCAST,
    CMD_WHEN_RECEIVE,
    CMD_WHEN_KEY_PRESSED,


    CMD_DISTANCE_TO_MOUSE,
    CMD_DISTANCE_TO_SPRITE,
    CMD_ASK_AND_WAIT,
    CMD_KEY_PRESSED,


    CMD_OP_ADD,
    CMD_OP_SUB,
    CMD_OP_MUL,
    CMD_OP_DIV,
    CMD_PICK_RANDOM,
    CMD_JOIN_STRING,


    CMD_OP_MOD,
    CMD_OP_ROUND,
    CMD_OP_AND,
    CMD_OP_OR,
    CMD_OP_NOT,
    CMD_OP_ABS,
    CMD_OP_SQRT,


    CMD_PEN_ERASE_ALL,
    CMD_PEN_STAMP,

    CMD_PEN_DOWN,
    CMD_PEN_UP,
    CMD_PEN_SET_COLOR,
    CMD_PEN_SET_SIZE,
    CMD_PEN_CHANGE_SIZE,


    CMD_PLAY_SOUND,
    CMD_STOP_SOUNDS
};

struct Block {
    BlockType type;
    string nameStr;
    string serializeStr;
    SDL_Color color;
    bool isContainer = false;
    float numParam1 = 0;
    float numParam2 = 0;
    string strParam;
    string strParam2;
    Condition condition;
};

struct UIBlock {
    SDL_Rect rect;
    string label;
    Block blockData;
    SDL_Color color;
};

struct LoopFrame {
    int   startIdx;
    float duration;
    float timePassed;
};

struct ExecutionContext {
    int sceneX, sceneY, sceneW, sceneH;
    map<string, double>* vars;
    float timer;
    float wallTime;

    bool keyLeft  = false;
    bool keyRight = false;
    bool keyUp    = false;
    bool keyDown  = false;
    bool keySpace = false;


    int mouseX = 0, mouseY = 0;
    string broadcastChannel;
    bool   broadcastPending = false;
    string lastAnswer;

    vector<Sprite>* sprites   = nullptr;
    int*            activeIdx = nullptr;


    Mix_Chunk* soundMeow = nullptr;
    Mix_Chunk* soundHop  = nullptr;
};

struct ExecutionEngine {
    vector<Block> blocks;
    int pc;
    bool stopped;
    bool isWaiting;
    float waitTimer;
    int stepCount;
    vector<LoopFrame> loopStack;


    map<int, int> jumpTable;

    float glideStartX, glideStartY, glideTargetX, glideTargetY, glideTotal, glideLeft;
    bool isGliding;


    bool paused = false;
    SDL_Texture* penLayer = nullptr;
};


struct SoundClip {
    string name;
    Mix_Chunk* chunk = nullptr;
};

static vector<SoundClip> gSounds;

void Sound_Init() {
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
        cout << "[AUDIO] Mix_OpenAudio error: " << Mix_GetError() << "\n";
}

void Sound_LoadAll() {

    auto tryLoad = [](const string& name, vector<string> paths) -> Mix_Chunk* {
        for (auto& p : paths) {
            Mix_Chunk* c = Mix_LoadWAV(p.c_str());
            if (c) { cout << "[AUDIO] Loaded " << p << "\n"; return c; }
        }
        cout << "[AUDIO] Could not load sound '" << name << "'\n";
        return nullptr;
    };

    SoundClip meow, hop;
    meow.name  = "meow";
    meow.chunk = tryLoad("meow", {"meow.wav", "meow.mp3", "sounds/meow.wav", "sounds/meow.mp3"});
    gSounds.push_back(meow);

    hop.name  = "hop";
    hop.chunk = tryLoad("hop", {"hop.wav", "hop.mp3", "sounds/hop.wav", "sounds/hop.mp3"});
    gSounds.push_back(hop);
}

void Sound_Play(const string& name) {
    for (auto& sc : gSounds) {
        if (sc.name == name && sc.chunk) {
            Mix_PlayChannel(-1, sc.chunk, 0);
            return;
        }
    }
    cout << "[AUDIO] Sound not found: " << name << "\n";
}

void Sound_StopAll() {
    Mix_HaltChannel(-1);
}

void Sound_Quit() {
    for (auto& sc : gSounds) if (sc.chunk) Mix_FreeChunk(sc.chunk);
    gSounds.clear();
    Mix_CloseAudio();
}


bool ShowInputDialog(SDL_Renderer* ren, TTF_Font* font, int SW, int SH,
                     const string& prompt, const string& current, string& outStr);
int ShowPickerModal(SDL_Renderer* ren, TTF_Font* font, int SW, int SH,
                    const string& title, const vector<string>& options);


bool ShowInputDialog(SDL_Renderer* ren, TTF_Font* font, int SW, int SH,
                     const string& prompt, const string& current, string& outStr) {
    outStr = current;
    SDL_StartTextInput();

    bool done = false, confirmed = false;

    while (!done) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) { done = true; break; }
            if (ev.type == SDL_KEYDOWN) {
                if (ev.key.keysym.sym == SDLK_RETURN || ev.key.keysym.sym == SDLK_KP_ENTER) {
                    confirmed = true; done = true;
                } else if (ev.key.keysym.sym == SDLK_ESCAPE) {
                    done = true;
                } else if (ev.key.keysym.sym == SDLK_BACKSPACE) {
                    if (!outStr.empty()) outStr.pop_back();
                }
            }
            if (ev.type == SDL_TEXTINPUT) {
                outStr += ev.text.text;
            }
        }


        SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 160);
        SDL_Rect full = {0, 0, SW, SH};
        SDL_RenderFillRect(ren, &full);

        int dw = 420, dh = 160;
        SDL_Rect dlg = {SW/2 - dw/2, SH/2 - dh/2, dw, dh};
        SDL_SetRenderDrawColor(ren, 40, 44, 70, 255);
        SDL_RenderFillRect(ren, &dlg);
        SDL_SetRenderDrawColor(ren, 100, 120, 200, 255);
        SDL_RenderDrawRect(ren, &dlg);

        SDL_Color white = {255,255,255,255};
        SDL_Color yellow = {255,220,80,255};


        SDL_Rect pr = {dlg.x + 10, dlg.y + 15, dw - 20, 30};
        if (font) {
            SDL_Surface* s = TTF_RenderUTF8_Blended(font, prompt.c_str(), white);
            if (s) {
                SDL_Texture* t = SDL_CreateTextureFromSurface(ren, s);
                SDL_Rect tr = {pr.x, pr.y, s->w, s->h};
                SDL_RenderCopy(ren, t, nullptr, &tr);
                SDL_DestroyTexture(t); SDL_FreeSurface(s);
            }
        }


        SDL_Rect inp = {dlg.x + 10, dlg.y + 60, dw - 20, 40};
        SDL_SetRenderDrawColor(ren, 20, 22, 40, 255);
        SDL_RenderFillRect(ren, &inp);
        SDL_SetRenderDrawColor(ren, 100, 160, 255, 255);
        SDL_RenderDrawRect(ren, &inp);

        string display = outStr + "|";
        if (font) {
            SDL_Surface* s = TTF_RenderUTF8_Blended(font, display.c_str(), yellow);
            if (s) {
                SDL_Texture* t = SDL_CreateTextureFromSurface(ren, s);
                SDL_Rect tr = {inp.x+8, inp.y+8, min(s->w, inp.w-16), s->h};
                SDL_RenderCopy(ren, t, nullptr, &tr);
                SDL_DestroyTexture(t); SDL_FreeSurface(s);
            }
        }


        SDL_Rect hr = {dlg.x+10, dlg.y+115, dw-20, 25};
        SDL_Color grey = {160,160,160,255};
        if (font) {
            SDL_Surface* s = TTF_RenderUTF8_Blended(font, "Enter = confirm   Esc = cancel", grey);
            if (s) {
                SDL_Texture* t = SDL_CreateTextureFromSurface(ren, s);
                SDL_Rect tr = {hr.x, hr.y, s->w, s->h};
                SDL_RenderCopy(ren, t, nullptr, &tr);
                SDL_DestroyTexture(t); SDL_FreeSurface(s);
            }
        }

        SDL_RenderPresent(ren);
    }
    SDL_StopTextInput();
    return confirmed;
}


TextureData LoadTexture(SDL_Renderer* renderer, string path) {
    TextureData td = {nullptr, 0, 0};
    SDL_Surface* surf = IMG_Load(path.c_str());
    if (!surf) {
        surf = SDL_CreateRGBSurface(0, 50, 50, 32, 0, 0, 0, 0);
        SDL_FillRect(surf, nullptr, SDL_MapRGB(surf->format, 100, 180, 255));
    }
    td.tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    SDL_QueryTexture(td.tex, nullptr, nullptr, &td.w, &td.h);
    return td;
}

void DrawText(SDL_Renderer* r, TTF_Font* f, string text, SDL_Color c, SDL_Rect rect, bool leftAlign=false) {
    if (!f || text.empty()) return;
    SDL_Surface* s = TTF_RenderUTF8_Blended(f, text.c_str(), c);
    if (!s) return;
    SDL_Texture* t = SDL_CreateTextureFromSurface(r, s);
    int tw = min(s->w, rect.w - 6);
    int th = min(s->h, rect.h - 4);
    int tx = leftAlign ? rect.x + 6 : rect.x + (rect.w - tw) / 2;
    int ty = rect.y + (rect.h - th) / 2;
    SDL_Rect tr = {tx, ty, tw, th};
    SDL_RenderCopy(r, t, nullptr, &tr);
    SDL_DestroyTexture(t);
    SDL_FreeSurface(s);
}

void DrawButton(SDL_Renderer* ren, TTF_Font* font, SDL_Rect r, SDL_Color c, string lbl) {
    SDL_SetRenderDrawColor(ren, c.r, c.g, c.b, 255);
    SDL_RenderFillRect(ren, &r);
    SDL_SetRenderDrawColor(ren, 255, 255, 255, 50);
    SDL_RenderDrawRect(ren, &r);
    SDL_Color white = {255, 255, 255, 255};
    DrawText(ren, font, lbl, white, r);
}


void DrawPuzzleBlock(SDL_Renderer* ren, SDL_Rect r, SDL_Color c,
                     bool hasTopTab, bool hasBotSlot) {


    SDL_SetRenderDrawColor(ren, c.r, c.g, c.b, c.a);
    SDL_RenderFillRect(ren, &r);


    if (hasTopTab) {
        SDL_Rect tab = { r.x + TAB_OX, r.y - TAB_H, TAB_W, TAB_H + 2 };
        SDL_SetRenderDrawColor(ren, c.r, c.g, c.b, c.a);
        SDL_RenderFillRect(ren, &tab);

        SDL_SetRenderDrawColor(ren,
                               (Uint8)min(255, (int)c.r + 40),
                               (Uint8)min(255, (int)c.g + 40),
                               (Uint8)min(255, (int)c.b + 40), 200);
        SDL_Rect tabHighlight = { tab.x, tab.y, tab.w, 2 };
        SDL_RenderFillRect(ren, &tabHighlight);
    }


    if (hasBotSlot) {
        SDL_Rect slot = { r.x + TAB_OX, r.y + r.h - 2, TAB_W, TAB_H + 2 };
        SDL_SetRenderDrawColor(ren, c.r, c.g, c.b, c.a);
        SDL_RenderFillRect(ren, &slot);

        SDL_SetRenderDrawColor(ren,
                               (Uint8)max(0, (int)c.r - 30),
                               (Uint8)max(0, (int)c.g - 30),
                               (Uint8)max(0, (int)c.b - 30), 255);
        SDL_Rect slotShadow = { slot.x, slot.y + slot.h - 2, slot.w, 2 };
        SDL_RenderFillRect(ren, &slotShadow);
    }


    SDL_SetRenderDrawColor(ren, 0, 0, 0, 90);
    SDL_RenderDrawRect(ren, &r);


    if (hasTopTab) {
        SDL_Rect tab = { r.x + TAB_OX, r.y - TAB_H, TAB_W, TAB_H };
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 70);

        SDL_RenderDrawLine(ren, tab.x,          tab.y, tab.x,          tab.y + tab.h);
        SDL_RenderDrawLine(ren, tab.x,          tab.y, tab.x + tab.w,  tab.y);
        SDL_RenderDrawLine(ren, tab.x + tab.w,  tab.y, tab.x + tab.w,  tab.y + tab.h);
    }


    if (hasBotSlot) {
        SDL_Rect slot = { r.x + TAB_OX, r.y + r.h, TAB_W, TAB_H };
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 70);

        SDL_RenderDrawLine(ren, slot.x,          slot.y, slot.x,          slot.y + slot.h);
        SDL_RenderDrawLine(ren, slot.x,          slot.y + slot.h, slot.x + slot.w, slot.y + slot.h);
        SDL_RenderDrawLine(ren, slot.x + slot.w, slot.y, slot.x + slot.w, slot.y + slot.h);
    }


    SDL_SetRenderDrawColor(ren, 255, 255, 255, 40);
    SDL_RenderDrawLine(ren, r.x + 1, r.y + 1, r.x + r.w - 2, r.y + 1);
    SDL_RenderDrawLine(ren, r.x + 1, r.y + 1, r.x + 1,       r.y + r.h - 2);
}


static bool IsEndBlock(BlockType t) {
    return t == CMD_END_REPEAT || t == CMD_END_FOREVER_BOUNCE ||
           t == CMD_END_IF     || t == CMD_ELSE        || t == CMD_END_REPEAT_UNTIL  ||
           t == CMD_END_REPEAT_FOR_SECONDS;
}

void DrawBlockAuto(SDL_Renderer* ren, SDL_Rect r, SDL_Color c, const Block& b) {
    bool isEnd = IsEndBlock(b.type);
    bool hasTopTab  = !isEnd;
    bool hasBotSlot = !b.isContainer;
    DrawPuzzleBlock(ren, r, c, hasTopTab, hasBotSlot);
}


void Sprite_ClampToScene(Sprite& sp, int sx, int sy, int sw, int sh) {
    sp.x = max((float)sx, min(sp.x, (float)(sx + sw)));
    sp.y = max((float)sy, min(sp.y, (float)(sy + sh)));
}

void Sprite_Init(Sprite& sp, string n, TextureData tex, float x, float y) {
    sp.name = n; sp.x = x; sp.y = y; sp.startX = x; sp.startY = y;
    sp.direction = 90.f; sp.startDirection = 90.f;
    sp.sizePct = 100.f; sp.startSizePct = 100.f;
    sp.visible = true; sp.layer = 0; sp.currentCostume = 0;
    sp.beingDragged = false; sp.dragOffX = 0; sp.dragOffY = 0;
    sp.bubble.active = false; sp.bubble.timeLeft = -1.f;
    if (tex.tex) sp.costumes.push_back(tex);
}

SDL_Rect Sprite_DestRect(const Sprite& sp) {
    if (sp.costumes.empty()) return {(int)sp.x, (int)sp.y, 10, 10};
    TextureData tex = sp.costumes[sp.currentCostume];
    int w = (int)(tex.w * sp.sizePct / 100.f);
    int h = (int)(tex.h * sp.sizePct / 100.f);
    return {(int)(sp.x - w/2), (int)(sp.y - h/2), w, h};
}

bool Sprite_ContainsPoint(const Sprite& sp, int px, int py) {
    SDL_Rect rc = Sprite_DestRect(sp);
    return px >= rc.x && px <= rc.x + rc.w && py >= rc.y && py <= rc.y + rc.h;
}

void Sprite_UpdateBubble(Sprite& sp, float dt) {
    if (sp.bubble.active && sp.bubble.timeLeft > 0.f) {
        sp.bubble.timeLeft -= dt;
        if (sp.bubble.timeLeft <= 0.f) { sp.bubble.active = false; sp.bubble.text = ""; }
    }
}

void Sprite_RenderBubble(const Sprite& sp, SDL_Renderer* r, TTF_Font* font, SDL_Rect sr) {
    if (!font || !sp.bubble.active || sp.bubble.text.empty()) return;
    int bx = sr.x + sr.w + 5, by = sr.y - 40;
    int bw = (int)sp.bubble.text.size() * 9 + 20, bh = 34;
    SDL_Rect box = {bx, by, bw, bh};
    SDL_SetRenderDrawColor(r, 255, 255, 255, 230); SDL_RenderFillRect(r, &box);
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255); SDL_RenderDrawRect(r, &box);
    SDL_Color c = {0,0,0,255};
    SDL_Surface* s = TTF_RenderUTF8_Blended(font, sp.bubble.text.c_str(), c);
    if (s) {
        SDL_Texture* t = SDL_CreateTextureFromSurface(r, s);
        SDL_Rect tr = {bx+5, by+5, s->w, s->h};
        SDL_RenderCopy(r, t, nullptr, &tr);
        SDL_DestroyTexture(t); SDL_FreeSurface(s);
    }
}

void Sprite_Render(const Sprite& sp, SDL_Renderer* r, TTF_Font* font) {
    if (!sp.visible) return;
    SDL_Rect dst = Sprite_DestRect(sp);
    if (!sp.costumes.empty())
        SDL_RenderCopyEx(r, sp.costumes[sp.currentCostume].tex, nullptr, &dst, sp.direction - 90.0, nullptr, SDL_FLIP_NONE);
    else {
        SDL_SetRenderDrawColor(r, 100, 180, 255, 255);
        SDL_RenderFillRect(r, &dst);
    }
    if (sp.bubble.active && !sp.bubble.text.empty()) Sprite_RenderBubble(sp, r, font, dst);
}
