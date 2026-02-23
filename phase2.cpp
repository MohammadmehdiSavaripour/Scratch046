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

// FIX: Replaced iteration-count fields with time-based tracking for CMD_REPEAT
struct LoopFrame {
    int   startIdx;
    float duration;    // seconds the loop should run
    float timePassed;  // seconds elapsed since loop start
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
    // FIX: CMD_END_FOREVER removed
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


Block CreateBlock(BlockType type, string name, string serialize, SDL_Color color, bool isContainer = false) {
    Block b;
    b.type = type; b.nameStr = name; b.serializeStr = serialize;
    b.color = color; b.isContainer = isContainer;
    b.numParam1 = 0; b.numParam2 = 0;
    b.condition.type = COND_ALWAYS_TRUE;
    b.condition.op = ">";
    return b;
}

UIBlock CreateUIBlock(Block b) {
    UIBlock ub;
    ub.rect = {0, 0, BLOCK_W, BLOCK_H};
    ub.blockData = b; ub.label = b.nameStr; ub.color = b.color;
    return ub;
}


string BlockLabel(const Block& b) {
    switch (b.type) {
        case CMD_MOVE:         return "Move " + to_string((int)b.numParam1) + " steps";
        case CMD_TURN_RIGHT:   return "Turn Right " + to_string((int)b.numParam1) + "°";
        case CMD_TURN_LEFT:    return "Turn Left " + to_string((int)b.numParam1) + "°";
        case CMD_GOTO:         return "GoTo x:" + to_string((int)b.numParam1) + " y:" + to_string((int)b.numParam2);
        case CMD_CHANGE_X:     return "Change X by " + to_string((int)b.numParam1);
        case CMD_CHANGE_Y:     return "Change Y by " + to_string((int)b.numParam1);
        case CMD_SET_X:        return "Set X to " + to_string((int)b.numParam1);
        case CMD_SET_Y:        return "Set Y to " + to_string((int)b.numParam1);
        case CMD_POINT_DIR:    return "Point Dir " + to_string((int)b.numParam1) + "°";
        case CMD_SAY:          return "Say: " + b.strParam + " (" + to_string((int)b.numParam1) + "s)";
        case CMD_THINK:        return "Think: " + b.strParam + " (" + to_string((int)b.numParam1) + "s)";
        case CMD_SET_SIZE:     return "Set Size " + to_string((int)b.numParam1) + "%";
        case CMD_CHANGE_SIZE:  return "Change Size " + to_string((int)b.numParam1);
        case CMD_WAIT:         return "Wait " + to_string(b.numParam1).substr(0,4) + "s";
        case CMD_REPEAT:       return "Repeat " + to_string((int)b.numParam1);
        case CMD_GLIDE: {
            string dur = to_string(b.numParam1); dur = dur.substr(0, dur.find('.')+2);
            int ty = 0; try { ty = (int)stof(b.strParam); } catch(...) {}
            return "Glide " + dur + "s to x:" + to_string((int)b.numParam2) + " y:" + to_string(ty);
        }
        case CMD_SET_VAR:      return "Set " + b.strParam + " = " + to_string((int)b.numParam1);
        case CMD_CHANGE_VAR:   return "Change " + b.strParam + " += " + to_string((int)b.numParam1);
        case CMD_IF: {
            auto& c = b.condition;
            if (c.type == COND_VAR_CMP)    return "If " + c.varName + " " + c.op + " " + to_string((int)c.rhs);
            if (c.type == COND_X_CMP)      return "If x " + c.op + " " + to_string((int)c.rhs);
            if (c.type == COND_Y_CMP)      return "If y " + c.op + " " + to_string((int)c.rhs);
            if (c.type == COND_DIR_CMP)    return "If dir " + c.op + " " + to_string((int)c.rhs);
            if (c.type == COND_SIZE_CMP)   return "If size " + c.op + " " + to_string((int)c.rhs);
            if (c.type == COND_TIMER_CMP)  return "If timer " + c.op + " " + to_string((int)c.rhs);
            if (c.type == COND_TOUCHING_EDGE) return "If touching edge";
            if (c.type == COND_KEY_LEFT)   return "If <left key>";
            if (c.type == COND_KEY_RIGHT)  return "If <right key>";
            if (c.type == COND_KEY_UP)     return "If <up key>";
            if (c.type == COND_KEY_DOWN)   return "If <down key>";
            if (c.type == COND_KEY_SPACE)  return "If <space>";
            return "If (condition)";
        }
        case CMD_REPEAT_UNTIL: {
            auto& c = b.condition;
            if (c.type == COND_VAR_CMP)    return "Repeat Until " + c.varName + " " + c.op + " " + to_string((int)c.rhs);
            if (c.type == COND_X_CMP)      return "Repeat Until x " + c.op + " " + to_string((int)c.rhs);
            if (c.type == COND_Y_CMP)      return "Repeat Until y " + c.op + " " + to_string((int)c.rhs);
            if (c.type == COND_TOUCHING_EDGE) return "Repeat Until edge";
            if (c.type == COND_KEY_LEFT)   return "Repeat Until <left key>";
            if (c.type == COND_KEY_RIGHT)  return "Repeat Until <right key>";
            if (c.type == COND_KEY_UP)     return "Repeat Until <up key>";
            if (c.type == COND_KEY_DOWN)   return "Repeat Until <down key>";
            return "Repeat Until (cond)";
        }
        case CMD_WAIT_UNTIL: {
            auto& c = b.condition;
            if (c.type == COND_VAR_CMP)    return "Wait Until " + c.varName + " " + c.op + " " + to_string((int)c.rhs);
            if (c.type == COND_X_CMP)      return "Wait Until x " + c.op + " " + to_string((int)c.rhs);
            if (c.type == COND_TOUCHING_EDGE) return "Wait Until edge";
            if (c.type == COND_KEY_LEFT)   return "Wait Until <left key>";
            if (c.type == COND_KEY_RIGHT)  return "Wait Until <right key>";
            if (c.type == COND_KEY_UP)     return "Wait Until <up key>";
            if (c.type == COND_KEY_DOWN)   return "Wait Until <down key>";
            if (c.type == COND_KEY_SPACE)  return "Wait Until <space>";
            if (c.type == COND_TIMER_CMP)  return "Wait Until timer " + c.op + " " + to_string((int)c.rhs);
            return "Wait Until (cond)";
        }
        case CMD_GO_TO_RANDOM:    return "Go to random position";
        case CMD_SET_VAR_RANDOM:  return "Set " + b.strParam + " = random(" + to_string((int)b.numParam1) + "," + to_string((int)b.numParam2) + ")";
        case CMD_FOREVER_BOUNCE:  return "Forever (bounce)";
        case CMD_REPEAT_FOR_SECONDS: {

            string d = to_string(b.numParam1);
            d = d.substr(0, d.find('.') + 2);
            return "Repeat for " + d + "s";
        }
        case CMD_END_REPEAT_FOR_SECONDS: return "End Timed Loop";


        case CMD_BROADCAST:        return "Broadcast: " + b.strParam;
        case CMD_WHEN_RECEIVE:     return "When I receive: " + b.strParam;
        case CMD_WHEN_KEY_PRESSED: return "When [" + b.strParam + "] pressed";


        case CMD_DISTANCE_TO_MOUSE:   return "Dist to mouse -> " + b.strParam;
        case CMD_DISTANCE_TO_SPRITE:  return "Dist to " + b.strParam2 + " -> " + b.strParam;
        case CMD_ASK_AND_WAIT:        return "Ask: " + b.strParam;
        case CMD_KEY_PRESSED:         return "Key [" + b.strParam2 + "] -> " + b.strParam;


        case CMD_OP_ADD:    return (b.strParam.empty() ? to_string((int)b.numParam1) : b.strParam)
                                   + " + " + to_string((int)b.numParam2) + " -> " + b.strParam2;
        case CMD_OP_SUB:    return (b.strParam.empty() ? to_string((int)b.numParam1) : b.strParam)
                                   + " - " + to_string((int)b.numParam2) + " -> " + b.strParam2;
        case CMD_OP_MUL:    return (b.strParam.empty() ? to_string((int)b.numParam1) : b.strParam)
                                   + " x " + to_string((int)b.numParam2) + " -> " + b.strParam2;
        case CMD_OP_DIV:    return (b.strParam.empty() ? to_string((int)b.numParam1) : b.strParam)
                                   + " / " + to_string((int)b.numParam2) + " -> " + b.strParam2;
        case CMD_PICK_RANDOM: return "Random " + to_string((int)b.numParam1)
                                     + "-" + to_string((int)b.numParam2) + " -> " + b.strParam2;
        case CMD_JOIN_STRING: return "Join [" + b.strParam + "][" + b.nameStr + "] -> " + b.strParam2;


        case CMD_OP_MOD:   return (b.strParam.empty() ? to_string((int)b.numParam1) : b.strParam)
                                  + " mod " + to_string((int)b.numParam2) + " -> " + b.strParam2;
        case CMD_OP_ROUND: return "round(" + (b.strParam.empty() ? to_string((int)b.numParam1) : b.strParam)
                                  + ") -> " + b.strParam2;
        case CMD_OP_AND:   return (b.strParam.empty() ? to_string((int)b.numParam1) : b.strParam)
                                  + " AND " + to_string((int)b.numParam2) + " -> " + b.strParam2;
        case CMD_OP_OR:    return (b.strParam.empty() ? to_string((int)b.numParam1) : b.strParam)
                                  + " OR " + to_string((int)b.numParam2) + " -> " + b.strParam2;
        case CMD_OP_NOT:   return "NOT(" + (b.strParam.empty() ? to_string((int)b.numParam1) : b.strParam)
                                  + ") -> " + b.strParam2;
        case CMD_OP_ABS:   return "abs(" + (b.strParam.empty() ? to_string((int)b.numParam1) : b.strParam)
                                  + ") -> " + b.strParam2;
        case CMD_OP_SQRT:  return "sqrt(" + (b.strParam.empty() ? to_string((int)b.numParam1) : b.strParam)
                                  + ") -> " + b.strParam2;


        case CMD_PEN_ERASE_ALL:  return "Pen: Erase All";
        case CMD_PEN_STAMP:      return "Pen: Stamp";

        case CMD_PEN_DOWN:       return "Pen: Pen Down";
        case CMD_PEN_UP:         return "Pen: Pen Up";
        case CMD_PEN_SET_COLOR:  return "Pen: Color R:" + to_string((int)b.numParam1)
                                        + " G:" + to_string((int)b.numParam2)
                                        + " B:" + to_string((int)b.condition.rhs);
        case CMD_PEN_SET_SIZE:   return "Pen: Set Size " + to_string((int)b.numParam1);
        case CMD_PEN_CHANGE_SIZE:return "Pen: Change Size " + to_string((int)b.numParam1);


        case CMD_PLAY_SOUND:   return "Play Sound: " + b.strParam;
        case CMD_STOP_SOUNDS:  return "Stop All Sounds";

        default: return b.nameStr;
    }
}


void Engine_Reset(ExecutionEngine& eng) {
    eng.pc = 0;
    eng.loopStack.clear();
    eng.waitTimer = 0.f;
    eng.isWaiting = false;
    eng.stopped = false;
    eng.stepCount = 0;
    eng.isGliding = false;
}


void Engine_PreProcess(ExecutionEngine& eng) {
    eng.jumpTable.clear();

    int N = (int)eng.blocks.size();

    struct IfFrame { int ifIdx; int elseIdx; };
    vector<IfFrame> stack;

    for (int i = 0; i < N; i++) {
        BlockType t = eng.blocks[i].type;

        if (t == CMD_IF) {
            stack.push_back({i, -1});
        }
        else if (t == CMD_ELSE) {
            if (!stack.empty()) {
                IfFrame& top = stack.back();
                eng.jumpTable[top.ifIdx] = i;
                top.elseIdx = i;
            }
        }
        else if (t == CMD_END_IF) {
            if (!stack.empty()) {
                IfFrame top = stack.back();
                stack.pop_back();
                if (top.elseIdx >= 0)
                    eng.jumpTable[top.elseIdx] = i;
                else
                    eng.jumpTable[top.ifIdx] = i;
            }
        }
    }


    for (auto& fr : stack) {
        if (eng.jumpTable.find(fr.ifIdx) == eng.jumpTable.end())
            eng.jumpTable[fr.ifIdx] = N;
        if (fr.elseIdx >= 0 &&
            eng.jumpTable.find(fr.elseIdx) == eng.jumpTable.end())
            eng.jumpTable[fr.elseIdx] = N;
    }
}

void Engine_LoadBlocks(ExecutionEngine& eng, const vector<UIBlock>& ubs) {
    eng.blocks.clear();
    for (auto& ub : ubs) eng.blocks.push_back(ub.blockData);
    Engine_PreProcess(eng);
    Engine_Reset(eng);
}


bool Sprite_TouchingEdge(const Sprite& sp, const ExecutionContext& ctx) {


    return (sp.x <= (float)ctx.sceneX) ||
           (sp.x >= (float)(ctx.sceneX + ctx.sceneW)) ||
           (sp.y <= (float)ctx.sceneY) ||
           (sp.y >= (float)(ctx.sceneY + ctx.sceneH));
}


bool EvalCondition(const Condition& cond, const Sprite& sprite, ExecutionContext& ctx) {
    if (cond.type == COND_TOUCHING_EDGE) return Sprite_TouchingEdge(sprite, ctx);
    if (cond.type == COND_ALWAYS_TRUE)   return true;
    if (cond.type == COND_ALWAYS_FALSE)  return false;
    if (cond.type == COND_KEY_LEFT)      return ctx.keyLeft;
    if (cond.type == COND_KEY_RIGHT)     return ctx.keyRight;
    if (cond.type == COND_KEY_UP)        return ctx.keyUp;
    if (cond.type == COND_KEY_DOWN)      return ctx.keyDown;
    if (cond.type == COND_KEY_SPACE)     return ctx.keySpace;

    double lhs = 0.0;
    switch (cond.type) {
        case COND_VAR_CMP:
            if (ctx.vars && ctx.vars->count(cond.varName))
                lhs = (*ctx.vars)[cond.varName];
            break;
        case COND_X_CMP:     lhs = sprite.x - ctx.sceneX; break;
        case COND_Y_CMP:     lhs = sprite.y - ctx.sceneY; break;
        case COND_DIR_CMP:   lhs = sprite.direction;      break;
        case COND_SIZE_CMP:  lhs = sprite.sizePct;        break;
        case COND_TIMER_CMP: lhs = ctx.timer;             break;
        default: break;
    }
    if (cond.op == "<")  return lhs <  cond.rhs;
    if (cond.op == ">")  return lhs >  cond.rhs;
    if (cond.op == "==") return lhs == cond.rhs;
    if (cond.op == "!=") return lhs != cond.rhs;
    if (cond.op == "<=") return lhs <= cond.rhs;
    if (cond.op == ">=") return lhs >= cond.rhs;
    return false;
}


bool Engine_RunStep(ExecutionEngine& eng, Sprite& sprite, ExecutionContext& ctx, float dt) {
    if (eng.stopped) return false;
    if (eng.paused)  return true;


    auto PenDraw = [&](float x0, float y0, float x1, float y1) {
        if (!eng.penLayer || !sprite.isPenDown) return;
        SDL_Renderer* r = SDL_GetRenderer(SDL_GetWindowFromID(1));
        if (!r) return;
        SDL_SetRenderTarget(r, eng.penLayer);
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(r,
                               sprite.penColor.r,
                               sprite.penColor.g,
                               sprite.penColor.b,
                               sprite.penColor.a);
        int half = (int)(sprite.penSize * 0.5f);
        half = max(0, half);

        for (int ox = -half; ox <= half; ox++) {
            for (int oy = -half; oy <= half; oy++) {
                SDL_RenderDrawLine(r,
                                   (int)x0 + ox, (int)y0 + oy,
                                   (int)x1 + ox, (int)y1 + oy);
            }
        }
        SDL_SetRenderTarget(r, nullptr);
    };


    if (eng.isGliding) {
        float oldX = sprite.x, oldY = sprite.y;
        eng.glideLeft -= dt;
        float t = (eng.glideTotal > 0.f)
                  ? (1.0f - max(0.f, eng.glideLeft / eng.glideTotal))
                  : 1.f;
        sprite.x = eng.glideStartX + (eng.glideTargetX - eng.glideStartX) * t;
        sprite.y = eng.glideStartY + (eng.glideTargetY - eng.glideStartY) * t;

        PenDraw(oldX, oldY, sprite.x, sprite.y);
        if (eng.glideLeft <= 0.f) {
            sprite.x = eng.glideTargetX;
            sprite.y = eng.glideTargetY;
            Sprite_ClampToScene(sprite, ctx.sceneX, ctx.sceneY, ctx.sceneW, ctx.sceneH);
            eng.isGliding = false;
            eng.pc++;
        }
        return true;
    }


    if (eng.isWaiting) {
        eng.waitTimer -= dt;
        if (eng.waitTimer > 0.f) return true;
        eng.isWaiting = false;
        eng.pc++;

    }


    // FIX: Advance the active loop's timer every frame using dt
    if (!eng.loopStack.empty()) {
        eng.loopStack.back().timePassed += dt;
    }

    const int MAX_STRUCT = 512;
    int budget = MAX_STRUCT;

    while (!eng.stopped && budget-- > 0) {

        if (eng.pc < 0 || eng.pc >= (int)eng.blocks.size()) {

            if (!eng.loopStack.empty()) {
                eng.pc = eng.loopStack.back().startIdx;
                return true;
            }

            eng.stopped = true;
            return false;
        }

        Block& blk = eng.blocks[eng.pc];

        switch (blk.type) {


            case CMD_STOP_ALL:
                eng.stopped = true;
                return false;

            case CMD_WAIT:
                eng.isWaiting = true;
                eng.waitTimer  = blk.numParam1;
                return true;

            case CMD_GLIDE:
                eng.isGliding   = true;
                eng.glideStartX = sprite.x;
                eng.glideStartY = sprite.y;
                eng.glideTargetX = (float)ctx.sceneX + blk.numParam2;
                eng.glideTargetY = (float)ctx.sceneY + atof(blk.strParam.c_str());
                eng.glideTotal   = max(0.001f, blk.numParam1);
                eng.glideLeft    = eng.glideTotal;
                return true;


            case CMD_IF:
                if (EvalCondition(blk.condition, sprite, ctx)) {
                    eng.pc++;
                } else {


                    if (eng.jumpTable.count(eng.pc)) {
                        int dest = eng.jumpTable[eng.pc];

                        if (dest < (int)eng.blocks.size() &&
                            eng.blocks[dest].type == CMD_ELSE)
                            eng.pc = dest + 1;
                        else
                            eng.pc = dest;
                    } else {

                        eng.pc = (int)eng.blocks.size();
                    }
                }
                break;

            case CMD_ELSE:


                if (eng.jumpTable.count(eng.pc))
                    eng.pc = eng.jumpTable[eng.pc];
                else
                    eng.pc++;


                break;

            case CMD_END_IF:
                eng.pc++;
                break;


            case CMD_REPEAT: {
                // FIX: Time-based repeat — push frame only once, then loop until timePassed >= duration
                bool alreadyTracked = (!eng.loopStack.empty() &&
                                       eng.loopStack.back().startIdx == eng.pc);
                if (!alreadyTracked) {
                    LoopFrame lf;
                    lf.startIdx   = eng.pc;
                    lf.duration   = blk.numParam1;  // FIX: duration in seconds, not iteration count
                    lf.timePassed = 0.0f;
                    eng.loopStack.push_back(lf);
                }
                eng.pc++;
                break;
            }

            case CMD_END_REPEAT: {
                // FIX: Jump back if time not yet elapsed; exit loop if duration is up
                if (!eng.loopStack.empty() &&
                    eng.loopStack.back().timePassed < eng.loopStack.back().duration) {
                    // Time not up — loop back to inside the repeat block
                    eng.pc = eng.loopStack.back().startIdx + 1;
                } else {
                    // Time is up — pop the frame and continue past the loop
                    if (!eng.loopStack.empty()) eng.loopStack.pop_back();
                    eng.pc++;
                }
                return true;
            }


            case CMD_FOREVER: {
                bool alreadyTracked = (!eng.loopStack.empty() &&
                                       eng.loopStack.back().startIdx == eng.pc);
                if (!alreadyTracked) {

                    LoopFrame lf;
                    lf.startIdx   = eng.pc;
                    lf.duration   = -1.f;  // FIX: infinite, no duration used
                    lf.timePassed = 0.0f;
                    eng.loopStack.push_back(lf);
                    eng.pc++;
                    break;
                } else {

                    eng.pc++;
                    return true;
                }
            }

                // FIX: CMD_END_FOREVER removed — CMD_FOREVER now loops back via loopStack at pc bounds check

            case CMD_FOREVER_BOUNCE: {
                bool alreadyTracked = (!eng.loopStack.empty() &&
                                       eng.loopStack.back().startIdx == eng.pc);
                if (!alreadyTracked) {
                    LoopFrame lf;
                    lf.startIdx   = eng.pc;
                    lf.duration   = -1.f;  // FIX: infinite, no duration used
                    lf.timePassed = 0.0f;
                    eng.loopStack.push_back(lf);
                    eng.pc++;
                } else {

                    float left   = (float)ctx.sceneX;
                    float right  = (float)(ctx.sceneX + ctx.sceneW);
                    float top    = (float)ctx.sceneY;
                    float bottom = (float)(ctx.sceneY + ctx.sceneH);
                    float rad2   = (sprite.direction - 90.f) * (float)M_PI / 180.f;
                    float vx = cosf(rad2), vy = sinf(rad2);
                    bool hitH = (sprite.x <= left || sprite.x >= right);
                    bool hitV = (sprite.y <= top  || sprite.y >= bottom);
                    if (hitH) vx = -vx;
                    if (hitV) vy = -vy;
                    if (hitH || hitV)
                        sprite.direction = fmodf(atan2f(vy, vx) * 180.f / (float)M_PI + 90.f + 360.f, 360.f);

                    sprite.x = max(left, min(sprite.x, right));
                    sprite.y = max(top,  min(sprite.y, bottom));
                    eng.pc++;
                    return true;
                }
                break;
            }

            case CMD_END_FOREVER_BOUNCE: {
                if (!eng.loopStack.empty())
                    eng.pc = eng.loopStack.back().startIdx;
                else
                    eng.pc++;
                return true;
            }


            case CMD_REPEAT_UNTIL: {
                bool alreadyTracked = (!eng.loopStack.empty() &&
                                       eng.loopStack.back().startIdx == eng.pc);
                if (!alreadyTracked) {
                    LoopFrame lf;
                    lf.startIdx   = eng.pc;
                    lf.duration   = -1.f;  // FIX: not time-based, unused
                    lf.timePassed = 0.0f;
                    eng.loopStack.push_back(lf);
                }
                if (EvalCondition(blk.condition, sprite, ctx)) {

                    if (eng.jumpTable.count(eng.pc))
                        eng.pc = eng.jumpTable[eng.pc] + 1;
                    else
                        eng.pc++;
                    eng.loopStack.pop_back();
                } else {
                    eng.pc++;
                }
                break;
            }

            case CMD_END_REPEAT_UNTIL: {
                if (!eng.loopStack.empty())
                    eng.pc = eng.loopStack.back().startIdx;
                else
                    eng.pc++;
                return true;
            }


            case CMD_REPEAT_FOR_SECONDS: {
                bool alreadyTracked = (!eng.loopStack.empty() &&
                                       eng.loopStack.back().startIdx == eng.pc);
                if (!alreadyTracked) {
                    // FIX: Use duration/timePassed instead of loopStartTime
                    LoopFrame lf;
                    lf.startIdx   = eng.pc;
                    lf.duration   = blk.numParam1;
                    lf.timePassed = 0.0f;
                    eng.loopStack.push_back(lf);
                    eng.pc++;
                    break;
                } else {

                    eng.pc++;
                    return true;
                }
            }

            case CMD_END_REPEAT_FOR_SECONDS: {
                if (!eng.loopStack.empty()) {
                    // FIX: Use timePassed instead of ctx.wallTime - loopStartTime
                    if (eng.loopStack.back().timePassed >= eng.loopStack.back().duration) {

                        eng.loopStack.pop_back();
                        eng.pc++;
                    } else {

                        eng.pc = eng.loopStack.back().startIdx;
                    }
                } else {
                    eng.pc++;
                }
                return true;
            }


            case CMD_MOVE: {
                float oldX = sprite.x, oldY = sprite.y;
                float rad = (sprite.direction - 90.f) * (float)M_PI / 180.f;
                sprite.x += blk.numParam1 * cosf(rad);
                sprite.y += blk.numParam1 * sinf(rad);
                Sprite_ClampToScene(sprite, ctx.sceneX, ctx.sceneY, ctx.sceneW, ctx.sceneH);
                PenDraw(oldX, oldY, sprite.x, sprite.y);
                eng.pc++; return true;
            }
            case CMD_TURN_RIGHT:
                sprite.direction = fmodf(sprite.direction + blk.numParam1 + 360.f, 360.f);
                eng.pc++; return true;
            case CMD_TURN_LEFT:
                sprite.direction = fmodf(sprite.direction - blk.numParam1 + 360.f, 360.f);
                eng.pc++; return true;

            case CMD_GOTO: {
                float oldX = sprite.x, oldY = sprite.y;
                sprite.x = ctx.sceneX + blk.numParam1;
                sprite.y = ctx.sceneY + blk.numParam2;
                Sprite_ClampToScene(sprite, ctx.sceneX, ctx.sceneY, ctx.sceneW, ctx.sceneH);
                PenDraw(oldX, oldY, sprite.x, sprite.y);
                eng.pc++; return true;
            }
            case CMD_CHANGE_X: {
                float oldX = sprite.x, oldY = sprite.y;
                sprite.x += blk.numParam1;
                Sprite_ClampToScene(sprite, ctx.sceneX, ctx.sceneY, ctx.sceneW, ctx.sceneH);
                PenDraw(oldX, oldY, sprite.x, sprite.y);
                eng.pc++; return true;
            }
            case CMD_CHANGE_Y: {
                float oldX = sprite.x, oldY = sprite.y;
                sprite.y += blk.numParam1;
                Sprite_ClampToScene(sprite, ctx.sceneX, ctx.sceneY, ctx.sceneW, ctx.sceneH);
                PenDraw(oldX, oldY, sprite.x, sprite.y);
                eng.pc++; return true;
            }
            case CMD_SET_X: {
                float oldX = sprite.x, oldY = sprite.y;
                sprite.x = ctx.sceneX + blk.numParam1;
                Sprite_ClampToScene(sprite, ctx.sceneX, ctx.sceneY, ctx.sceneW, ctx.sceneH);
                PenDraw(oldX, oldY, sprite.x, sprite.y);
                eng.pc++; return true;
            }
            case CMD_SET_Y: {
                float oldX = sprite.x, oldY = sprite.y;
                sprite.y = ctx.sceneY + blk.numParam1;
                Sprite_ClampToScene(sprite, ctx.sceneX, ctx.sceneY, ctx.sceneW, ctx.sceneH);
                PenDraw(oldX, oldY, sprite.x, sprite.y);
                eng.pc++; return true;
            }
            case CMD_POINT_DIR:
                sprite.direction = fmodf(blk.numParam1 + 360.f, 360.f);
                eng.pc++; return true;
            case CMD_POINT_TOWARDS_CENTER: {
                float cx = ctx.sceneX + ctx.sceneW / 2.f;
                float cy = ctx.sceneY + ctx.sceneH / 2.f;
                float angle = atan2f(cy - sprite.y, cx - sprite.x) * 180.f / (float)M_PI + 90.f;
                sprite.direction = fmodf(angle + 360.f, 360.f);
                eng.pc++; return true;
            }
            case CMD_BOUNCE_IF_EDGE: {


                float hw = 5.f, hh = 5.f;
                if (!sprite.costumes.empty()) {
                    TextureData& td = sprite.costumes[sprite.currentCostume];
                    hw = td.w * sprite.sizePct / 200.f;
                    hh = td.h * sprite.sizePct / 200.f;
                }
                float left   = (float)ctx.sceneX;
                float right  = (float)(ctx.sceneX + ctx.sceneW);
                float top    = (float)ctx.sceneY;
                float bottom = (float)(ctx.sceneY + ctx.sceneH);

                if ((sprite.x - hw) < left)   sprite.x = left   + hw;
                if ((sprite.x + hw) > right)   sprite.x = right  - hw;
                if ((sprite.y - hh) < top)     sprite.y = top    + hh;
                if ((sprite.y + hh) > bottom)  sprite.y = bottom - hh;

                eng.pc++; return true;
            }
            case CMD_SAY:
            case CMD_THINK:
                sprite.bubble.text     = blk.strParam;
                sprite.bubble.active   = !blk.strParam.empty();
                sprite.bubble.isThink  = (blk.type == CMD_THINK);
                sprite.bubble.timeLeft = blk.numParam1;
                eng.pc++; return true;
            case CMD_SHOW: sprite.visible = true;  eng.pc++; return true;
            case CMD_HIDE: sprite.visible = false; eng.pc++; return true;
            case CMD_SET_SIZE:
                sprite.sizePct = max(5.f, min(blk.numParam1, 500.f));
                eng.pc++; return true;
            case CMD_CHANGE_SIZE:
                sprite.sizePct = max(5.f, min(sprite.sizePct + blk.numParam1, 500.f));
                eng.pc++; return true;
            case CMD_NEXT_COSTUME:
                if (!sprite.costumes.empty())
                    sprite.currentCostume = (sprite.currentCostume + 1) % (int)sprite.costumes.size();
                eng.pc++; return true;

            case CMD_RESET:
                sprite.x = sprite.startX; sprite.y = sprite.startY;
                sprite.direction = sprite.startDirection;
                sprite.sizePct   = sprite.startSizePct;
                sprite.visible   = true;
                sprite.bubble.active = false;
                sprite.currentCostume = 0;
                sprite.isPenDown = false;
                sprite.penColor  = {200, 0, 0, 255};
                sprite.penSize   = 2.0f;
                eng.pc++; return true;
            case CMD_SET_VAR:
                if (ctx.vars) (*ctx.vars)[blk.strParam] = blk.numParam1;
                eng.pc++; return true;
            case CMD_CHANGE_VAR:
                if (ctx.vars) (*ctx.vars)[blk.strParam] += blk.numParam1;
                eng.pc++; return true;
            case CMD_RESET_TIMER:
                ctx.timer = 0.f;
                eng.pc++; return true;


            case CMD_WAIT_UNTIL:
                if (EvalCondition(blk.condition, sprite, ctx)) {
                    eng.pc++;
                }

                return true;


            case CMD_GO_TO_RANDOM:
                sprite.x = (float)(ctx.sceneX + rand() % max(1, ctx.sceneW));
                sprite.y = (float)(ctx.sceneY + rand() % max(1, ctx.sceneH));
                Sprite_ClampToScene(sprite, ctx.sceneX, ctx.sceneY, ctx.sceneW, ctx.sceneH);
                eng.pc++; return true;


            case CMD_SET_VAR_RANDOM:
                if (ctx.vars && !blk.strParam.empty()) {
                    int lo = (int)blk.numParam1, hi = (int)blk.numParam2;
                    if (lo > hi) swap(lo, hi);
                    (*ctx.vars)[blk.strParam] = lo + (hi > lo ? rand() % (hi - lo + 1) : 0);
                }
                eng.pc++; return true;


            case CMD_BROADCAST:
                ctx.broadcastChannel = blk.strParam;
                ctx.broadcastPending = true;
                eng.pc++; return true;


            case CMD_WHEN_RECEIVE:
                if (ctx.broadcastPending && ctx.broadcastChannel == blk.strParam) {
                    ctx.broadcastPending = false;
                    eng.pc++;
                }

                return true;


            case CMD_WHEN_KEY_PRESSED: {
                SDL_Scancode sc = SDL_GetScancodeFromName(blk.strParam.c_str());
                const Uint8* ks = SDL_GetKeyboardState(nullptr);
                if (sc != SDL_SCANCODE_UNKNOWN && ks[sc])
                    eng.pc++;
                return true;
            }


            case CMD_DISTANCE_TO_MOUSE: {
                float dx = sprite.x - (float)ctx.mouseX;
                float dy = sprite.y - (float)ctx.mouseY;
                if (ctx.vars) (*ctx.vars)[blk.strParam] = (double)sqrtf(dx*dx + dy*dy);
                eng.pc++; return true;
            }


            case CMD_DISTANCE_TO_SPRITE: {
                if (ctx.vars && ctx.sprites) {
                    float tx = sprite.x, ty = sprite.y;
                    for (auto& s : *ctx.sprites) {
                        if (s.name == blk.strParam2) { tx = s.x; ty = s.y; break; }
                    }
                    float dx = sprite.x - tx, dy = sprite.y - ty;
                    (*ctx.vars)[blk.strParam] = (double)sqrtf(dx*dx + dy*dy);
                }
                eng.pc++; return true;
            }


            case CMD_ASK_AND_WAIT: {


                ctx.lastAnswer   = blk.strParam;
                eng.isWaiting    = true;
                eng.waitTimer    = -999.f;
                return true;
            }


            case CMD_KEY_PRESSED: {
                if (ctx.vars) {
                    SDL_Scancode sc = SDL_GetScancodeFromName(blk.strParam2.c_str());
                    const Uint8* ks = SDL_GetKeyboardState(nullptr);
                    bool held = (sc != SDL_SCANCODE_UNKNOWN) && ks[sc];
                    (*ctx.vars)[blk.strParam] = held ? 1.0 : 0.0;
                }
                eng.pc++; return true;
            }


            case CMD_OP_ADD: {
                if (ctx.vars) {
                    double a = blk.strParam.empty()  ? blk.numParam1 : (*ctx.vars)[blk.strParam];
                    double b = blk.numParam2;
                    (*ctx.vars)[blk.strParam2] = a + b;
                }
                eng.pc++; return true;
            }
            case CMD_OP_SUB: {
                if (ctx.vars) {
                    double a = blk.strParam.empty() ? blk.numParam1 : (*ctx.vars)[blk.strParam];
                    double b = blk.numParam2;
                    (*ctx.vars)[blk.strParam2] = a - b;
                }
                eng.pc++; return true;
            }
            case CMD_OP_MUL: {
                if (ctx.vars) {
                    double a = blk.strParam.empty() ? blk.numParam1 : (*ctx.vars)[blk.strParam];
                    double b = blk.numParam2;
                    (*ctx.vars)[blk.strParam2] = a * b;
                }
                eng.pc++; return true;
            }
            case CMD_OP_DIV: {
                if (ctx.vars) {
                    double a = blk.strParam.empty() ? blk.numParam1 : (*ctx.vars)[blk.strParam];
                    double b = blk.numParam2;
                    (*ctx.vars)[blk.strParam2] = (b != 0.0) ? a / b : 0.0;
                }
                eng.pc++; return true;
            }
            case CMD_PICK_RANDOM: {
                if (ctx.vars) {
                    double lo = blk.numParam1, hi = blk.numParam2;
                    if (lo > hi) swap(lo, hi);
                    double range = hi - lo;
                    double val   = lo + (range > 0 ? ((double)rand() / RAND_MAX) * range : 0.0);
                    (*ctx.vars)[blk.strParam2] = val;
                }
                eng.pc++; return true;
            }
            case CMD_JOIN_STRING: {


                if (ctx.vars) {
                    string a = blk.strParam;
                    string b = blk.nameStr;
                    (*ctx.vars)[blk.strParam2];

                    ctx.lastAnswer = a + b;


                    (*ctx.vars)[blk.strParam2] = (double)(a.size() + b.size());
                }
                eng.pc++; return true;
            }


            case CMD_OP_MOD: {
                if (ctx.vars) {
                    double a = blk.strParam.empty() ? blk.numParam1 : (*ctx.vars)[blk.strParam];
                    double b = blk.numParam2;
                    (*ctx.vars)[blk.strParam2] = (b != 0.0) ? fmod(a, b) : 0.0;
                }
                eng.pc++; return true;
            }
            case CMD_OP_ROUND: {
                if (ctx.vars) {
                    double a = blk.strParam.empty() ? blk.numParam1 : (*ctx.vars)[blk.strParam];
                    (*ctx.vars)[blk.strParam2] = round(a);
                }
                eng.pc++; return true;
            }
            case CMD_OP_AND: {
                if (ctx.vars) {
                    double a = blk.strParam.empty() ? blk.numParam1 : (*ctx.vars)[blk.strParam];
                    double b = blk.numParam2;
                    (*ctx.vars)[blk.strParam2] = (a != 0.0 && b != 0.0) ? 1.0 : 0.0;
                }
                eng.pc++; return true;
            }
            case CMD_OP_OR: {
                if (ctx.vars) {
                    double a = blk.strParam.empty() ? blk.numParam1 : (*ctx.vars)[blk.strParam];
                    double b = blk.numParam2;
                    (*ctx.vars)[blk.strParam2] = (a != 0.0 || b != 0.0) ? 1.0 : 0.0;
                }
                eng.pc++; return true;
            }
            case CMD_OP_NOT: {
                if (ctx.vars) {
                    double a = blk.strParam.empty() ? blk.numParam1 : (*ctx.vars)[blk.strParam];
                    (*ctx.vars)[blk.strParam2] = (a == 0.0) ? 1.0 : 0.0;
                }
                eng.pc++; return true;
            }
            case CMD_OP_ABS: {
                if (ctx.vars) {
                    double a = blk.strParam.empty() ? blk.numParam1 : (*ctx.vars)[blk.strParam];
                    (*ctx.vars)[blk.strParam2] = fabs(a);
                }
                eng.pc++; return true;
            }
            case CMD_OP_SQRT: {
                if (ctx.vars) {
                    double a = blk.strParam.empty() ? blk.numParam1 : (*ctx.vars)[blk.strParam];
                    (*ctx.vars)[blk.strParam2] = (a >= 0.0) ? sqrt(a) : 0.0;
                }
                eng.pc++; return true;
            }


            case CMD_PEN_ERASE_ALL: {
                if (eng.penLayer) {
                    SDL_Renderer* r = SDL_GetRenderer(SDL_GetWindowFromID(1));
                    if (r) {
                        SDL_SetRenderTarget(r, eng.penLayer);
                        SDL_SetRenderDrawColor(r, 0, 0, 0, 0);
                        SDL_RenderClear(r);
                        SDL_SetRenderTarget(r, nullptr);
                    }
                }
                eng.pc++; return true;
            }


            case CMD_PEN_STAMP: {
                if (eng.penLayer && !sprite.costumes.empty()) {
                    SDL_Renderer* r = SDL_GetRenderer(SDL_GetWindowFromID(1));
                    if (r) {
                        SDL_SetRenderTarget(r, eng.penLayer);
                        SDL_Rect dst = Sprite_DestRect(sprite);
                        TextureData& td = sprite.costumes[sprite.currentCostume];
                        SDL_RenderCopyEx(r, td.tex, nullptr, &dst,
                                         sprite.direction - 90.0, nullptr, SDL_FLIP_NONE);
                        SDL_SetRenderTarget(r, nullptr);
                    }
                }
                eng.pc++; return true;
            }


            case CMD_PEN_DOWN:
                sprite.isPenDown = true;
                eng.pc++; return true;

            case CMD_PEN_UP:
                sprite.isPenDown = false;
                eng.pc++; return true;


            case CMD_PEN_SET_COLOR:
                sprite.penColor.r = (Uint8)max(0.f, min(255.f, blk.numParam1));
                sprite.penColor.g = (Uint8)max(0.f, min(255.f, blk.numParam2));
                sprite.penColor.b = (Uint8)max(0.0,  min(255.0, blk.condition.rhs));
                sprite.penColor.a = 255;
                eng.pc++; return true;

            case CMD_PEN_SET_SIZE:
                sprite.penSize = max(1.f, min(blk.numParam1, 50.f));
                eng.pc++; return true;

            case CMD_PEN_CHANGE_SIZE:
                sprite.penSize = max(1.f, min(sprite.penSize + blk.numParam1, 50.f));
                eng.pc++; return true;


            case CMD_PLAY_SOUND:
                Sound_Play(blk.strParam);
                eng.pc++; return true;

            case CMD_STOP_SOUNDS:
                Sound_StopAll();
                eng.pc++; return true;

            default:
                eng.pc++;
                break;
        }
    }


    if (eng.pc >= (int)eng.blocks.size()) {
        eng.stopped = true;
        return false;
    }
    return true;
}


vector<UIBlock> BuildPalette() {
    vector<UIBlock> pal;
    int y = TOOLBAR_H + 8;
    auto Add = [&](Block b) {
        UIBlock ub = CreateUIBlock(b);
        ub.label = BlockLabel(b);
        ub.rect = {6, y, PALETTE_WIDTH - 12, BLOCK_H};
        pal.push_back(ub);
        y += BLOCK_H + 4;
    };

    SDL_Color cBlue   = {70,130,220,255};
    SDL_Color cPurple = {110,80,200,255};
    SDL_Color cOrange = {220,140,30,255};
    SDL_Color cRed    = {180,60,60,255};
    SDL_Color cCyan   = {30,160,160,255};
    SDL_Color cBrown  = {160,100,20,255};
    SDL_Color cGreen  = {40,160,80,255};


    Block b;
    b = CreateBlock(CMD_MOVE, "", "Move 10", cBlue); b.numParam1 = 10; Add(b);
    b = CreateBlock(CMD_TURN_RIGHT, "", "TurnRight 15", cBlue); b.numParam1 = 15; Add(b);
    b = CreateBlock(CMD_TURN_LEFT,  "", "TurnLeft 15",  cBlue); b.numParam1 = 15; Add(b);
    b = CreateBlock(CMD_GOTO, "", "GoTo 0 0", cBlue); b.numParam1 = 0; b.numParam2 = 0; Add(b);
    b = CreateBlock(CMD_CHANGE_X, "", "ChangeX 10", cBlue); b.numParam1 = 10; Add(b);
    b = CreateBlock(CMD_CHANGE_Y, "", "ChangeY 10",  cBlue); b.numParam1 = 10; Add(b);
    b = CreateBlock(CMD_SET_X, "", "SetX 0", cBlue); b.numParam1 = 0; Add(b);
    b = CreateBlock(CMD_SET_Y, "", "SetY 0", cBlue); b.numParam1 = 0; Add(b);
    b = CreateBlock(CMD_POINT_DIR, "", "PointDir 90", cBlue); b.numParam1 = 90; Add(b);
    b = CreateBlock(CMD_POINT_TOWARDS_CENTER, "Point to Center", "PointCenter", cBlue); Add(b);
    b = CreateBlock(CMD_BOUNCE_IF_EDGE, "Bounce if on Edge", "BounceEdge", cBlue); Add(b);

    b = CreateBlock(CMD_GLIDE, "", "Glide 1 0 0", cBlue);
    b.numParam1 = 1.0f; b.numParam2 = 0; b.strParam = "0";
    b.nameStr = "Glide 1s to x:0 y:0"; Add(b);


    b = CreateBlock(CMD_SAY, "", "Say Hello! 2", cPurple); b.strParam = "Hello!"; b.numParam1 = 2; Add(b);
    b = CreateBlock(CMD_THINK, "", "Think Hmm... 2", cPurple); b.strParam = "Hmm..."; b.numParam1 = 2; Add(b);
    b = CreateBlock(CMD_SHOW, "Show", "Show", cPurple); Add(b);
    b = CreateBlock(CMD_HIDE, "Hide", "Hide", cPurple); Add(b);
    b = CreateBlock(CMD_SET_SIZE, "", "SetSize 100", cPurple); b.numParam1 = 100; Add(b);
    b = CreateBlock(CMD_CHANGE_SIZE, "", "ChangeSize 10", cPurple); b.numParam1 = 10; Add(b);
    b = CreateBlock(CMD_NEXT_COSTUME, "Next Costume", "NextCostume", cPurple); Add(b);


    b = CreateBlock(CMD_WAIT, "", "Wait 1.0", cOrange); b.numParam1 = 1.0; Add(b);
    b = CreateBlock(CMD_REPEAT, "", "Repeat 10", cOrange, true); b.numParam1 = 10; Add(b);
    b = CreateBlock(CMD_END_REPEAT, "End Repeat", "EndRepeat", cBrown); Add(b);

    b = CreateBlock(CMD_REPEAT_FOR_SECONDS, "", "RepeatForSecs 5", cOrange, true);
    b.numParam1 = 5.0f; Add(b);
    b = CreateBlock(CMD_END_REPEAT_FOR_SECONDS, "End Timed Loop", "EndRepeatForSecs", cBrown); Add(b);
    b = CreateBlock(CMD_FOREVER_BOUNCE, "Forever (bounce)", "ForeverBounce", cOrange, true); Add(b);
    b = CreateBlock(CMD_END_FOREVER_BOUNCE, "End Forever (bounce)", "EndForeverBounce", cBrown); Add(b);
    b = CreateBlock(CMD_STOP_ALL, "Stop All", "StopAll", cOrange); Add(b);


    { Block ib = CreateBlock(CMD_IF, "", "If x > 100", cGreen, true);
        ib.condition = {COND_X_CMP, "", ">", 100}; Add(ib); }
    { Block ib = CreateBlock(CMD_IF, "", "If x < 100", cGreen, true);
        ib.condition = {COND_X_CMP, "", "<", 100}; Add(ib); }
    { Block ib = CreateBlock(CMD_IF, "", "If y > 100", cGreen, true);
        ib.condition = {COND_Y_CMP, "", ">", 100}; Add(ib); }
    { Block ib = CreateBlock(CMD_IF, "", "If y < 100", cGreen, true);
        ib.condition = {COND_Y_CMP, "", "<", 100}; Add(ib); }
    { Block ib = CreateBlock(CMD_IF, "", "If dir == 90", cGreen, true);
        ib.condition = {COND_DIR_CMP, "", "==", 90}; Add(ib); }
    { Block ib = CreateBlock(CMD_IF, "", "If size > 50", cGreen, true);
        ib.condition = {COND_SIZE_CMP, "", ">", 50}; Add(ib); }
    { Block ib = CreateBlock(CMD_IF, "", "If timer > 5", cGreen, true);
        ib.condition = {COND_TIMER_CMP, "", ">", 5}; Add(ib); }
    { Block ib = CreateBlock(CMD_IF, "", "If touching edge", cGreen, true);
        ib.condition = {COND_TOUCHING_EDGE}; Add(ib); }
    b = CreateBlock(CMD_ELSE, "Else", "Else", cBrown); Add(b);
    b = CreateBlock(CMD_END_IF, "End If", "EndIf", cBrown); Add(b);


    { Block ib = CreateBlock(CMD_REPEAT_UNTIL, "", "RepeatUntil x > 200", cOrange, true);
        ib.condition = {COND_X_CMP, "", ">", 200}; Add(ib); }
    { Block ib = CreateBlock(CMD_REPEAT_UNTIL, "", "RepeatUntil edge", cOrange, true);
        ib.condition = {COND_TOUCHING_EDGE}; Add(ib); }
    b = CreateBlock(CMD_END_REPEAT_UNTIL, "End RepeatUntil", "EndRepeatUntil", cBrown); Add(b);


    b = CreateBlock(CMD_RESET_TIMER, "Reset Timer", "ResetTimer", cCyan); Add(b);
    b = CreateBlock(CMD_RESET, "Reset Sprite", "Reset", cRed); Add(b);
    b = CreateBlock(CMD_GO_TO_RANDOM, "Go to random pos", "GoToRandom", cBlue); Add(b);


    { Block wu = CreateBlock(CMD_WAIT_UNTIL, "", "WaitUntilEdge", cOrange);
        wu.condition = {COND_TOUCHING_EDGE}; Add(wu); }
    { Block wu = CreateBlock(CMD_WAIT_UNTIL, "", "WaitUntilTimer", cOrange);
        wu.condition = {COND_TIMER_CMP, "", ">", 5}; Add(wu); }


    b = CreateBlock(CMD_SET_VAR, "", "SetVar x 0", cRed); b.strParam = "x"; b.numParam1 = 0; Add(b);
    b = CreateBlock(CMD_CHANGE_VAR, "", "ChangeVar x 1", cRed); b.strParam = "x"; b.numParam1 = 1; Add(b);
    b = CreateBlock(CMD_SET_VAR, "", "SetVar score 0", cRed); b.strParam = "score"; b.numParam1 = 0; Add(b);
    b = CreateBlock(CMD_CHANGE_VAR, "", "ChangeVar score 1", cRed); b.strParam = "score"; b.numParam1 = 1; Add(b);
    { Block rv = CreateBlock(CMD_SET_VAR_RANDOM, "", "SetVar x random", cRed);
        rv.strParam = "x"; rv.numParam1 = 1; rv.numParam2 = 100; Add(rv); }


    { Block ib = CreateBlock(CMD_IF, "", "If left key", cGreen, true);
        ib.condition = {COND_KEY_LEFT}; Add(ib); }
    { Block ib = CreateBlock(CMD_IF, "", "If right key", cGreen, true);
        ib.condition = {COND_KEY_RIGHT}; Add(ib); }
    { Block ib = CreateBlock(CMD_IF, "", "If up key", cGreen, true);
        ib.condition = {COND_KEY_UP}; Add(ib); }
    { Block ib = CreateBlock(CMD_IF, "", "If down key", cGreen, true);
        ib.condition = {COND_KEY_DOWN}; Add(ib); }

    { Block wu = CreateBlock(CMD_WAIT_UNTIL, "", "WaitUntilLeft", cOrange);
        wu.condition = {COND_KEY_LEFT}; Add(wu); }
    { Block wu = CreateBlock(CMD_WAIT_UNTIL, "", "WaitUntilRight", cOrange);
        wu.condition = {COND_KEY_RIGHT}; Add(wu); }
    { Block wu = CreateBlock(CMD_WAIT_UNTIL, "", "WaitUntilSpace", cOrange);
        wu.condition = {COND_KEY_SPACE}; Add(wu); }


    SDL_Color cYellow    = {220, 180, 30, 255};
    b = CreateBlock(CMD_BROADCAST, "", "Broadcast hello", cYellow);
    b.strParam = "hello"; Add(b);
    b = CreateBlock(CMD_WHEN_RECEIVE, "", "WhenReceive hello", cYellow);
    b.strParam = "hello"; Add(b);
    b = CreateBlock(CMD_WHEN_KEY_PRESSED, "", "WhenKey space", cYellow);
    b.strParam = "space"; Add(b);
    b = CreateBlock(CMD_WHEN_KEY_PRESSED, "", "WhenKey Left", cYellow);
    b.strParam = "Left"; Add(b);
    b = CreateBlock(CMD_WHEN_KEY_PRESSED, "", "WhenKey Right", cYellow);
    b.strParam = "Right"; Add(b);


    SDL_Color cLightBlue = {60, 160, 210, 255};
    b = CreateBlock(CMD_DISTANCE_TO_MOUSE, "", "DistMouse", cLightBlue);
    b.strParam = "dist"; Add(b);
    b = CreateBlock(CMD_DISTANCE_TO_SPRITE, "", "DistSprite", cLightBlue);
    b.strParam = "dist"; b.strParam2 = "Cat"; Add(b);
    b = CreateBlock(CMD_ASK_AND_WAIT, "", "Ask", cLightBlue);
    b.strParam = "What's your name?"; Add(b);
    b = CreateBlock(CMD_KEY_PRESSED, "", "KeyPressed", cLightBlue);
    b.strParam = "keyHeld"; b.strParam2 = "space"; Add(b);


    SDL_Color cLime = {80, 190, 60, 255};
    b = CreateBlock(CMD_OP_ADD, "", "OpAdd", cLime);
    b.strParam = "score"; b.numParam2 = 1; b.strParam2 = "result"; Add(b);
    b = CreateBlock(CMD_OP_SUB, "", "OpSub", cLime);
    b.strParam = "score"; b.numParam2 = 1; b.strParam2 = "result"; Add(b);
    b = CreateBlock(CMD_OP_MUL, "", "OpMul", cLime);
    b.strParam = "score"; b.numParam2 = 2; b.strParam2 = "result"; Add(b);
    b = CreateBlock(CMD_OP_DIV, "", "OpDiv", cLime);
    b.strParam = "score"; b.numParam2 = 2; b.strParam2 = "result"; Add(b);
    b = CreateBlock(CMD_PICK_RANDOM, "", "PickRandom", cLime);
    b.numParam1 = 1; b.numParam2 = 10; b.strParam2 = "result"; Add(b);
    b = CreateBlock(CMD_JOIN_STRING, "", "JoinStr", cLime);
    b.strParam = "Hello "; b.nameStr = "World"; b.strParam2 = "joined"; Add(b);

    b = CreateBlock(CMD_OP_MOD,   "", "OpMod",   cLime);
    b.strParam = "score"; b.numParam2 = 3; b.strParam2 = "result"; Add(b);
    b = CreateBlock(CMD_OP_ROUND, "", "OpRound", cLime);
    b.strParam = "score"; b.strParam2 = "result"; Add(b);
    b = CreateBlock(CMD_OP_AND,   "", "OpAnd",   cLime);
    b.strParam = "x"; b.numParam2 = 1; b.strParam2 = "result"; Add(b);
    b = CreateBlock(CMD_OP_OR,    "", "OpOr",    cLime);
    b.strParam = "x"; b.numParam2 = 0; b.strParam2 = "result"; Add(b);
    b = CreateBlock(CMD_OP_NOT,   "", "OpNot",   cLime);
    b.strParam = "x"; b.strParam2 = "result"; Add(b);
    b = CreateBlock(CMD_OP_ABS,   "", "OpAbs",   cLime);
    b.strParam = "x"; b.strParam2 = "result"; Add(b);
    b = CreateBlock(CMD_OP_SQRT,  "", "OpSqrt",  cLime);
    b.strParam = "x"; b.strParam2 = "result"; Add(b);


    SDL_Color cDarkGreen = {20, 130, 60, 255};
    b = CreateBlock(CMD_PEN_ERASE_ALL, "Pen: Erase All", "PenEraseAll", cDarkGreen); Add(b);
    b = CreateBlock(CMD_PEN_STAMP,     "Pen: Stamp",     "PenStamp",    cDarkGreen); Add(b);

    b = CreateBlock(CMD_PEN_DOWN,      "Pen: Pen Down",  "PenDown",     cDarkGreen); Add(b);
    b = CreateBlock(CMD_PEN_UP,        "Pen: Pen Up",    "PenUp",       cDarkGreen); Add(b);
    b = CreateBlock(CMD_PEN_SET_COLOR, "", "PenSetColor", cDarkGreen);
    b.numParam1 = 200; b.numParam2 = 0; b.condition.rhs = 0; Add(b);
    b = CreateBlock(CMD_PEN_SET_SIZE,  "", "PenSetSize",  cDarkGreen);
    b.numParam1 = 2; Add(b);
    b = CreateBlock(CMD_PEN_CHANGE_SIZE,"","PenChgSize",  cDarkGreen);
    b.numParam1 = 1; Add(b);


    SDL_Color cSound = {120, 60, 160, 255};
    b = CreateBlock(CMD_PLAY_SOUND, "", "PlaySound meow", cSound);
    b.strParam = "meow"; Add(b);
    b = CreateBlock(CMD_PLAY_SOUND, "", "PlaySound hop", cSound);
    b.strParam = "hop"; Add(b);
    b = CreateBlock(CMD_STOP_SOUNDS, "Stop All Sounds", "StopSounds", cSound); Add(b);


    for (auto& ub : pal) ub.label = BlockLabel(ub.blockData);

    return pal;
}


void SaveProject(const vector<UIBlock>& userBlocks, const vector<Sprite>& sprites, const map<string, double>& vars) {
    ofstream f("project_save.txt");
    if (!f.is_open()) { cout << "[SAVE] Could not open file\n"; return; }
    f << "SPRITES " << sprites.size() << "\n";
    for (const auto& sp : sprites)
        f << sp.name << " " << sp.x << " " << sp.y << " " << sp.direction << " "
          << sp.sizePct << " " << (sp.visible?1:0) << " " << sp.currentCostume << "\n";
    f << "VARS " << vars.size() << "\n";
    for (const auto& kv : vars) f << kv.first << " " << kv.second << "\n";
    f << "BLOCKS " << userBlocks.size() << "\n";
    for (const auto& ub : userBlocks) {
        const Block& b = ub.blockData;
        f << b.type << " " << b.numParam1 << " " << b.numParam2 << " "
          << b.color.r << " " << b.color.g << " " << b.color.b << " "
          << "\"" << b.strParam << "\" \"" << b.strParam2 << "\" "
          << "\"" << b.nameStr << "\" \"" << b.serializeStr << "\" "
          << b.condition.type << " \"" << b.condition.varName << "\" "
          << "\"" << b.condition.op << "\" " << b.condition.rhs << "\n";
    }
    f.close();
    cout << "[SAVE] Saved\n";
}

bool LoadProject(vector<UIBlock>& userBlocks, vector<Sprite>& sprites, map<string, double>& vars) {
    ifstream f("project_save.txt");
    if (!f.is_open()) { cout << "[LOAD] Not found\n"; return false; }
    string tag; int count;
    f >> tag >> count;
    if (tag == "SPRITES" && count == (int)sprites.size()) {
        for (int i = 0; i < count; i++) {
            string name; float x, y, dir, sz; int vis, cos;
            f >> name >> x >> y >> dir >> sz >> vis >> cos;
            if (i < (int)sprites.size()) {
                sprites[i].x=x; sprites[i].y=y; sprites[i].direction=dir;
                sprites[i].sizePct=sz; sprites[i].visible=(vis==1);
                sprites[i].currentCostume = cos % max(1,(int)sprites[i].costumes.size());
            }
        }
    }
    f >> tag >> count;
    if (tag == "VARS") {
        for (int i = 0; i < count; i++) { string key; double val; f >> key >> val; vars[key] = val; }
    }
    f >> tag >> count;
    userBlocks.clear();
    if (tag == "BLOCKS") {
        auto readQ = [&](string& out) {
            char c; string s;
            while (f.get(c) && c != '"');
            while (f.get(c) && c != '"') s += c;
            out = s;
        };
        for (int i = 0; i < count; i++) {
            int btype; float n1, n2; int cr, cg, cb;
            string sp1, sp2, nm, ser;
            f >> btype >> n1 >> n2 >> cr >> cg >> cb;
            readQ(sp1); readQ(sp2); readQ(nm); readQ(ser);
            int ctype; string cvar, cop; double crhs;
            f >> ctype; readQ(cvar); readQ(cop); f >> crhs;
            SDL_Color col = {(Uint8)cr,(Uint8)cg,(Uint8)cb,255};
            Block b = CreateBlock((BlockType)btype, nm, ser, col);
            b.numParam1=n1; b.numParam2=n2; b.strParam=sp1; b.strParam2=sp2;
            b.condition = {(CondType)ctype, cvar, cop, crhs};
            UIBlock ub = CreateUIBlock(b);
            ub.label = BlockLabel(b);
            int yy = TOOLBAR_H + 8 + (int)userBlocks.size() * (BLOCK_H + 6);
            ub.rect = {PALETTE_WIDTH + 8, yy, PALETTE_WIDTH - 12, BLOCK_H};
            userBlocks.push_back(ub);
        }
    }
    f.close();
    cout << "[LOAD] Loaded\n";
    return true;
}


void EditBlock(UIBlock& ub, SDL_Renderer* ren, TTF_Font* font, int SW, int SH) {
    Block& b = ub.blockData;
    string res;
    switch (b.type) {
        case CMD_MOVE:
        case CMD_CHANGE_X: case CMD_CHANGE_Y:
        case CMD_SET_X:    case CMD_SET_Y:
        case CMD_TURN_RIGHT: case CMD_TURN_LEFT:
        case CMD_POINT_DIR:
        case CMD_SET_SIZE: case CMD_CHANGE_SIZE:
        case CMD_WAIT:
        case CMD_REPEAT:
        case CMD_REPEAT_FOR_SECONDS: {
            string cur = to_string(b.numParam1); cur = cur.substr(0, cur.find('.')+2);
            if (ShowInputDialog(ren, font, SW, SH, "Enter value:", cur, res)) {
                try { b.numParam1 = stof(res); } catch (...) {}
            }
            break;
        }
        case CMD_GOTO: {
            string cur = to_string((int)b.numParam1);
            if (ShowInputDialog(ren, font, SW, SH, "Enter X:", cur, res)) {
                try { b.numParam1 = stof(res); } catch (...) {}
            }
            string cur2 = to_string((int)b.numParam2);
            if (ShowInputDialog(ren, font, SW, SH, "Enter Y:", cur2, res)) {
                try { b.numParam2 = stof(res); } catch (...) {}
            }
            break;
        }
        case CMD_GLIDE: {
            string cur = to_string(b.numParam1); cur=cur.substr(0,cur.find('.')+2);
            if (ShowInputDialog(ren, font, SW, SH, "Glide duration (s):", cur, res))
                try { b.numParam1 = stof(res); } catch(...) {}
            cur = to_string((int)b.numParam2);
            if (ShowInputDialog(ren, font, SW, SH, "Target X:", cur, res))
                try { b.numParam2 = stof(res); } catch(...) {}
            cur = b.strParam.empty() ? "0" : b.strParam;
            if (ShowInputDialog(ren, font, SW, SH, "Target Y:", cur, res))
                b.strParam = res;
            break;
        }
        case CMD_SAY: case CMD_THINK: {
            if (ShowInputDialog(ren, font, SW, SH, "Enter message:", b.strParam, res))
                b.strParam = res;
            string cur2 = to_string(b.numParam1); cur2=cur2.substr(0,cur2.find('.')+2);
            if (ShowInputDialog(ren, font, SW, SH, "Duration (s):", cur2, res))
                try { b.numParam1 = stof(res); } catch(...) {}
            break;
        }
        case CMD_SET_VAR: case CMD_CHANGE_VAR: {
            if (ShowInputDialog(ren, font, SW, SH, "Variable name:", b.strParam, res))
                b.strParam = res;
            string cur2 = to_string((int)b.numParam1);
            if (ShowInputDialog(ren, font, SW, SH, "Value:", cur2, res))
                try { b.numParam1 = stof(res); } catch(...) {}
            break;
        }
        case CMD_IF: case CMD_REPEAT_UNTIL: {


            string typeStr;
            if (ShowInputDialog(ren, font, SW, SH, "Compare: x / y / dir / size / timer / var / edge", "", typeStr)) {
                Condition& c = b.condition;
                if (typeStr == "x")     { c.type = COND_X_CMP; c.varName = ""; }
                else if (typeStr == "y"){ c.type = COND_Y_CMP; c.varName = ""; }
                else if (typeStr == "dir"){ c.type = COND_DIR_CMP; c.varName = ""; }
                else if (typeStr == "size"){ c.type = COND_SIZE_CMP; c.varName = ""; }
                else if (typeStr == "timer"){ c.type = COND_TIMER_CMP; c.varName = ""; }
                else if (typeStr == "edge"){ c.type = COND_TOUCHING_EDGE; c.varName = ""; }
                else { c.type = COND_VAR_CMP; c.varName = typeStr; }

                if (c.type != COND_TOUCHING_EDGE) {
                    string opStr;
                    if (ShowInputDialog(ren, font, SW, SH, "Operator (< > == != <= >=):", c.op, opStr))
                        c.op = opStr;
                    string rhsStr;
                    if (ShowInputDialog(ren, font, SW, SH, "Compare to value:", to_string((int)c.rhs), rhsStr))
                        try { c.rhs = stod(rhsStr); } catch(...) {}
                }
            }
            break;
        }

        case CMD_BROADCAST:
        case CMD_WHEN_RECEIVE:
            if (ShowInputDialog(ren, font, SW, SH, "Message name:", b.strParam, res))
                b.strParam = res;
            break;
        case CMD_WHEN_KEY_PRESSED:
            if (ShowInputDialog(ren, font, SW, SH, "Key name (e.g. Left, Right, space, a):", b.strParam, res))
                b.strParam = res;
            break;


        case CMD_DISTANCE_TO_MOUSE:
            if (ShowInputDialog(ren, font, SW, SH, "Store distance in variable:", b.strParam, res))
                b.strParam = res;
            break;
        case CMD_DISTANCE_TO_SPRITE:
            if (ShowInputDialog(ren, font, SW, SH, "Store distance in variable:", b.strParam, res))
                b.strParam = res;
            if (ShowInputDialog(ren, font, SW, SH, "Target sprite name:", b.strParam2, res))
                b.strParam2 = res;
            break;
        case CMD_ASK_AND_WAIT:
            if (ShowInputDialog(ren, font, SW, SH, "Question to ask:", b.strParam, res))
                b.strParam = res;
            break;
        case CMD_KEY_PRESSED:
            if (ShowInputDialog(ren, font, SW, SH, "Store result (1/0) in variable:", b.strParam, res))
                b.strParam = res;
            if (ShowInputDialog(ren, font, SW, SH, "Key name (e.g. space, Left, a):", b.strParam2, res))
                b.strParam2 = res;
            break;


        case CMD_OP_ADD: case CMD_OP_SUB: case CMD_OP_MUL: case CMD_OP_DIV: {
            if (ShowInputDialog(ren, font, SW, SH, "Operand A (var name or leave blank for literal):", b.strParam, res))
                b.strParam = res;
            if (b.strParam.empty()) {
                string cur = to_string((int)b.numParam1);
                if (ShowInputDialog(ren, font, SW, SH, "Literal value for A:", cur, res))
                    try { b.numParam1 = stof(res); } catch(...) {}
            }
            string cur2 = to_string((int)b.numParam2);
            if (ShowInputDialog(ren, font, SW, SH, "Operand B (literal value):", cur2, res))
                try { b.numParam2 = stof(res); } catch(...) {}
            if (ShowInputDialog(ren, font, SW, SH, "Store result in variable:", b.strParam2, res))
                b.strParam2 = res;
            break;
        }
        case CMD_PICK_RANDOM: {
            string cur = to_string((int)b.numParam1);
            if (ShowInputDialog(ren, font, SW, SH, "Min value:", cur, res))
                try { b.numParam1 = stof(res); } catch(...) {}
            string cur2 = to_string((int)b.numParam2);
            if (ShowInputDialog(ren, font, SW, SH, "Max value:", cur2, res))
                try { b.numParam2 = stof(res); } catch(...) {}
            if (ShowInputDialog(ren, font, SW, SH, "Store result in variable:", b.strParam2, res))
                b.strParam2 = res;
            break;
        }
        case CMD_JOIN_STRING:
            if (ShowInputDialog(ren, font, SW, SH, "First string:", b.strParam, res))
                b.strParam = res;
            if (ShowInputDialog(ren, font, SW, SH, "Second string:", b.nameStr, res))
                b.nameStr = res;
            if (ShowInputDialog(ren, font, SW, SH, "Store length in variable:", b.strParam2, res))
                b.strParam2 = res;
            break;


        case CMD_OP_MOD: case CMD_OP_AND: case CMD_OP_OR: {
            if (ShowInputDialog(ren, font, SW, SH, "Operand A (var name or blank for literal):", b.strParam, res))
                b.strParam = res;
            if (b.strParam.empty()) {
                string cur = to_string((int)b.numParam1);
                if (ShowInputDialog(ren, font, SW, SH, "Literal value for A:", cur, res))
                    try { b.numParam1 = stof(res); } catch(...) {}
            }
            string cur2 = to_string((int)b.numParam2);
            if (ShowInputDialog(ren, font, SW, SH, "Operand B (literal):", cur2, res))
                try { b.numParam2 = stof(res); } catch(...) {}
            if (ShowInputDialog(ren, font, SW, SH, "Store result in variable:", b.strParam2, res))
                b.strParam2 = res;
            break;
        }
        case CMD_OP_ROUND: case CMD_OP_NOT: case CMD_OP_ABS: case CMD_OP_SQRT: {
            if (ShowInputDialog(ren, font, SW, SH, "Operand A (var name or blank for literal):", b.strParam, res))
                b.strParam = res;
            if (b.strParam.empty()) {
                string cur = to_string((int)b.numParam1);
                if (ShowInputDialog(ren, font, SW, SH, "Literal value for A:", cur, res))
                    try { b.numParam1 = stof(res); } catch(...) {}
            }
            if (ShowInputDialog(ren, font, SW, SH, "Store result in variable:", b.strParam2, res))
                b.strParam2 = res;
            break;
        }


        case CMD_PEN_DOWN:
        case CMD_PEN_UP:
        case CMD_PEN_ERASE_ALL:
        case CMD_PEN_STAMP:
            break;

        case CMD_PLAY_SOUND: {

            vector<string> opts = {"meow", "hop"};
            int choice = ShowPickerModal(ren, font, SW, SH, "  Choose Sound", opts);
            if (choice >= 0) b.strParam = opts[choice];
            break;
        }
        case CMD_STOP_SOUNDS:
            break;

        case CMD_PEN_SET_COLOR: {
            string cur = to_string((int)b.numParam1);
            if (ShowInputDialog(ren, font, SW, SH, "Red (0-255):", cur, res))
                try { b.numParam1 = max(0.f, min(255.f, stof(res))); } catch(...) {}
            cur = to_string((int)b.numParam2);
            if (ShowInputDialog(ren, font, SW, SH, "Green (0-255):", cur, res))
                try { b.numParam2 = max(0.f, min(255.f, stof(res))); } catch(...) {}
            cur = to_string((int)b.condition.rhs);
            if (ShowInputDialog(ren, font, SW, SH, "Blue (0-255):", cur, res))
                try { b.condition.rhs = max(0.0, min(255.0, stod(res))); } catch(...) {}
            break;
        }
        case CMD_PEN_SET_SIZE:
        case CMD_PEN_CHANGE_SIZE: {
            string cur = to_string((int)b.numParam1);
            if (ShowInputDialog(ren, font, SW, SH, "Pen size (1-50):", cur, res))
                try { b.numParam1 = max(1.f, min(50.f, stof(res))); } catch(...) {}
            break;
        }

        default: break;
    }

    ub.label = BlockLabel(b);
}


int ShowPickerModal(SDL_Renderer* ren, TTF_Font* font, int SW, int SH,
                    const string& title, const vector<string>& options) {
    const int ITEM_H   = 52;
    const int MODAL_W  = 360;
    const int HEADER_H = 50;
    const int PAD      = 12;
    int modalH = HEADER_H + (int)options.size() * (ITEM_H + PAD) + PAD;
    int mx0 = SW / 2 - MODAL_W / 2;
    int my0 = SH / 2 - modalH / 2;

    SDL_Color white   = {255, 255, 255, 255};
    SDL_Color black   = {0,   0,   0,   255};
    SDL_Color lblCol  = {220, 230, 255, 255};

    int result = -1;
    bool done  = false;

    while (!done) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) { done = true; break; }
            if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE) {
                done = true; break;
            }
            if (ev.type == SDL_MOUSEBUTTONDOWN && ev.button.button == SDL_BUTTON_LEFT) {
                int cx = ev.button.x, cy = ev.button.y;

                if (cx < mx0 || cx > mx0 + MODAL_W || cy < my0 || cy > my0 + modalH) {
                    done = true; break;
                }

                for (int i = 0; i < (int)options.size(); i++) {
                    int ry = my0 + HEADER_H + i * (ITEM_H + PAD) + PAD;
                    SDL_Rect row = {mx0 + PAD, ry, MODAL_W - PAD * 2, ITEM_H};
                    if (cx >= row.x && cx <= row.x + row.w && cy >= row.y && cy <= row.y + row.h) {
                        result = i; done = true; break;
                    }
                }
            }
        }


        SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 170);
        SDL_Rect full = {0, 0, SW, SH};
        SDL_RenderFillRect(ren, &full);


        SDL_Rect panel = {mx0, my0, MODAL_W, modalH};
        SDL_SetRenderDrawColor(ren, 35, 38, 60, 255);
        SDL_RenderFillRect(ren, &panel);
        SDL_SetRenderDrawColor(ren, 100, 120, 210, 255);
        SDL_RenderDrawRect(ren, &panel);


        SDL_Rect titleBar = {mx0, my0, MODAL_W, HEADER_H};
        SDL_SetRenderDrawColor(ren, 55, 60, 100, 255);
        SDL_RenderFillRect(ren, &titleBar);
        DrawText(ren, font, title, white, titleBar);


        int mx, my; SDL_GetMouseState(&mx, &my);
        for (int i = 0; i < (int)options.size(); i++) {
            int ry = my0 + HEADER_H + i * (ITEM_H + PAD) + PAD;
            SDL_Rect row = {mx0 + PAD, ry, MODAL_W - PAD * 2, ITEM_H};
            bool hovered = (mx >= row.x && mx <= row.x + row.w &&
                            my >= row.y && my <= row.y + row.h);
            SDL_SetRenderDrawColor(ren,
                                   hovered ? 80 : 50,
                                   hovered ? 90 : 58,
                                   hovered ? 160 : 100, 255);
            SDL_RenderFillRect(ren, &row);
            SDL_SetRenderDrawColor(ren,
                                   hovered ? 160 : 90,
                                   hovered ? 180 : 100,
                                   hovered ? 255 : 180, 255);
            SDL_RenderDrawRect(ren, &row);
            DrawText(ren, font, options[i], hovered ? white : lblCol, row);
        }

        SDL_RenderPresent(ren);
        SDL_Delay(16);
    }
    return result;
}


int main(int argc, char* argv[]) {
    srand((unsigned)time(nullptr));
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) { cout << SDL_GetError(); return 1; }
    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) == 0) { cout << IMG_GetError(); SDL_Quit(); return 1; }
    if (TTF_Init() == -1) { cout << TTF_GetError(); IMG_Quit(); SDL_Quit(); return 1; }
    Sound_Init();
    Sound_LoadAll();

    SDL_Window* win = SDL_CreateWindow("Scratch IDE", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                       0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP);
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    int SW, SH; SDL_GetWindowSize(win, &SW, &SH);

    TTF_Font* font = TTF_OpenFont("ARIAL.TTF", 14);
    if (!font) { cout << "[WARN] ARIAL.TTF not found: " << TTF_GetError() << "\n"; }


    SDL_Texture* penLayerTex = SDL_CreateTexture(ren,
                                                 SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, SW, SH);
    SDL_SetTextureBlendMode(penLayerTex, SDL_BLENDMODE_BLEND);

    SDL_SetRenderTarget(ren, penLayerTex);
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 0);
    SDL_RenderClear(ren);
    SDL_SetRenderTarget(ren, nullptr);

    int scriptPanelW = (SW - PALETTE_WIDTH) / 3;
    int scriptBlockW = scriptPanelW - 16;

    int sceneX = PALETTE_WIDTH + scriptPanelW;
    int sceneY = TOOLBAR_H;
    int sceneW = SW - sceneX;
    int sceneH = SH - TOOLBAR_H - PANEL_HEIGHT;

    TextureData defTex = LoadTexture(ren, "sprite.png");


    SDL_Texture* bgTexture = nullptr;

    vector<Sprite> sprites;
    int activeIndex = 0;
    Sprite sp;
    Sprite_Init(sp, "Cat", defTex, sceneX + sceneW / 2.f, sceneY + sceneH / 2.f);
    sp.sizePct = 30.f; sp.startSizePct = 30.f;
    sprites.push_back(sp);

    map<string, double> varStore;
    varStore["x"] = 0.0;
    varStore["y"] = 0.0;
    varStore["score"] = 0.0;

    ExecutionContext ctx;
    ctx.sceneX = sceneX; ctx.sceneY = sceneY;
    ctx.sceneW = sceneW; ctx.sceneH = sceneH;
    ctx.vars = &varStore; ctx.timer = 0.f; ctx.wallTime = 0.f;
    ctx.sprites    = &sprites;
    ctx.activeIdx  = &activeIndex;

    vector<UIBlock> palette = BuildPalette();
    vector<UIBlock> userBlocks;
    ExecutionEngine engine;
    engine.penLayer = penLayerTex;


    const int MAX_UNDO = 32;
    vector<vector<UIBlock>> undoStack;
    int undoPos = -1;
    auto undoPush = [&]() {

        if (undoPos < (int)undoStack.size() - 1)
            undoStack.erase(undoStack.begin() + undoPos + 1, undoStack.end());
        undoStack.push_back(userBlocks);
        if ((int)undoStack.size() > MAX_UNDO) undoStack.erase(undoStack.begin());
        undoPos = (int)undoStack.size() - 1;
    };
    undoPush();


    int bx = 10;
    SDL_Rect btnPlay         = {bx, 6, 100, 38}; bx += 106;
    SDL_Rect btnStop         = {bx, 6,  80, 38}; bx +=  86;
    SDL_Rect btnSave         = {bx, 6,  80, 38}; bx +=  86;
    SDL_Rect btnLoad         = {bx, 6,  80, 38}; bx +=  86;
    SDL_Rect btnClear        = {bx, 6,  70, 38}; bx +=  76;
    SDL_Rect btnUndo         = {bx, 6,  60, 38}; bx +=  66;
    SDL_Rect btnRedo         = {bx, 6,  60, 38}; bx +=  66;
    SDL_Rect btnAddSprite    = {bx, 6,  90, 38}; bx +=  96;
    SDL_Rect btnDeleteSprite = {bx, 6, 100, 38}; bx += 106;
    SDL_Rect btnChangeBg     = {bx, 6, 100, 38}; bx += 106;
    SDL_Rect btnClearBg      = {bx, 6,  90, 38}; bx +=  96;
    SDL_Rect btnPause        = {bx, 6,  90, 38}; bx +=  96;
    SDL_Rect btnQuit         = {SW - 86, 6, 78, 38};


    bool running = true, playing = false, draggingBlock = false;
    UIBlock draggedBlock;
    int blkOX = 0, blkOY = 0;
    Sprite* draggedSprite = nullptr;
    float stepTimer = 0.f;
    int palScrollY = 0, scriptScrollY = 0;

    auto lastTime = steady_clock::now();
    SDL_Color white = {255,255,255,255}, black = {0,0,0,255};

    while (running) {
        auto now = steady_clock::now();
        float dt = duration<float>(now - lastTime).count();
        lastTime = now;
        ctx.timer    += dt;
        ctx.wallTime += dt;

        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) running = false;

            if (ev.type == SDL_KEYDOWN) {
                auto k = ev.key.keysym.sym;
                bool ctrl = (ev.key.keysym.mod & KMOD_CTRL) != 0;
                bool shift = (ev.key.keysym.mod & KMOD_SHIFT) != 0;

                if (k == SDLK_ESCAPE) running = false;


                if (k == SDLK_LEFT)  ctx.keyLeft  = true;
                if (k == SDLK_RIGHT) ctx.keyRight = true;
                if (k == SDLK_UP)    ctx.keyUp    = true;
                if (k == SDLK_DOWN)  ctx.keyDown  = true;
                if (k == SDLK_SPACE && playing) ctx.keySpace = true;


                if (k == SDLK_SPACE && !playing) {
                    playing = true; Engine_LoadBlocks(engine, userBlocks); stepTimer = 0.f;
                }


                if (k == SDLK_DELETE && !userBlocks.empty() && !playing) {
                    userBlocks.pop_back();
                    Engine_LoadBlocks(engine, userBlocks);
                    undoPush();
                }


                if (ctrl && k == SDLK_z && !shift && undoPos > 0) {
                    undoPos--;
                    userBlocks = undoStack[undoPos];
                    Engine_LoadBlocks(engine, userBlocks);
                }

                if (ctrl && (k == SDLK_y || (k == SDLK_z && shift)) && undoPos < (int)undoStack.size() - 1) {
                    undoPos++;
                    userBlocks = undoStack[undoPos];
                    Engine_LoadBlocks(engine, userBlocks);
                }
            }

            if (ev.type == SDL_KEYUP) {
                auto k = ev.key.keysym.sym;
                if (k == SDLK_LEFT)  ctx.keyLeft  = false;
                if (k == SDLK_RIGHT) ctx.keyRight = false;
                if (k == SDLK_UP)    ctx.keyUp    = false;
                if (k == SDLK_DOWN)  ctx.keyDown  = false;
                if (k == SDLK_SPACE) ctx.keySpace = false;
            }

            if (ev.type == SDL_MOUSEWHEEL) {
                int mx, my; SDL_GetMouseState(&mx, &my);
                if (mx < PALETTE_WIDTH) palScrollY = max(0, palScrollY - ev.wheel.y * 20);
                else if (mx < PALETTE_WIDTH + scriptPanelW) scriptScrollY = max(0, scriptScrollY - ev.wheel.y * 20);
            }

            if (ev.type == SDL_MOUSEBUTTONDOWN) {
                int mx = ev.button.x, my = ev.button.y;
                auto inR = [](int x, int y, SDL_Rect r){ return x>=r.x && x<=r.x+r.w && y>=r.y && y<=r.y+r.h; };

                if (inR(mx, my, btnPlay))  { playing = true; engine.paused = false; Engine_LoadBlocks(engine, userBlocks); stepTimer = 0.f; }
                if (inR(mx, my, btnStop))  {
                    playing = false; engine.paused = false; Engine_Reset(engine);

                    if (penLayerTex) {
                        SDL_SetRenderTarget(ren, penLayerTex);
                        SDL_SetRenderDrawColor(ren, 0, 0, 0, 0);
                        SDL_RenderClear(ren);
                        SDL_SetRenderTarget(ren, nullptr);
                    }
                }
                if (inR(mx, my, btnPause) && playing) { engine.paused = !engine.paused; }
                if (inR(mx, my, btnClear)) { userBlocks.clear(); Engine_LoadBlocks(engine, userBlocks); undoPush(); }
                if (inR(mx, my, btnSave))  { SaveProject(userBlocks, sprites, varStore); }
                if (inR(mx, my, btnLoad))  { LoadProject(userBlocks, sprites, varStore); Engine_LoadBlocks(engine, userBlocks); undoPush(); }
                if (inR(mx, my, btnQuit))  { running = false; }
                if (inR(mx, my, btnUndo) && undoPos > 0) {
                    undoPos--; userBlocks = undoStack[undoPos]; Engine_LoadBlocks(engine, userBlocks);
                }
                if (inR(mx, my, btnRedo) && undoPos < (int)undoStack.size() - 1) {
                    undoPos++; userBlocks = undoStack[undoPos]; Engine_LoadBlocks(engine, userBlocks);
                }


                if (inR(mx, my, btnAddSprite) && !playing) {
                    vector<string> opts = {"sprite.png", "dog.png", "spider.png"};
                    int choice = ShowPickerModal(ren, font, SW, SH, "  Add Sprite", opts);
                    if (choice >= 0) {
                        TextureData td = LoadTexture(ren, opts[choice]);
                        Sprite ns;

                        string nm = opts[choice];
                        nm = nm.substr(0, nm.find('.'));
                        Sprite_Init(ns, nm, td,
                                    sceneX + sceneW / 2.f,
                                    sceneY + sceneH / 2.f);
                        ns.sizePct = 30.f; ns.startSizePct = 30.f;
                        sprites.push_back(ns);
                        activeIndex = (int)sprites.size() - 1;
                    }
                }


                if (inR(mx, my, btnDeleteSprite) && !sprites.empty()) {

                    playing = false;
                    Engine_Reset(engine);

                    if (draggedSprite == &sprites[activeIndex]) {
                        draggedSprite = nullptr;
                    }

                    for (auto& td : sprites[activeIndex].costumes)
                        if (td.tex) SDL_DestroyTexture(td.tex);
                    sprites.erase(sprites.begin() + activeIndex);

                    if (!sprites.empty())
                        activeIndex = max(0, min(activeIndex, (int)sprites.size() - 1));
                    else
                        activeIndex = 0;
                }


                if (inR(mx, my, btnChangeBg) && !playing) {
                    vector<string> opts = {"bg1.png", "bg2.png"};
                    int choice = ShowPickerModal(ren, font, SW, SH, "  Change Background", opts);
                    if (choice >= 0) {
                        if (bgTexture) { SDL_DestroyTexture(bgTexture); bgTexture = nullptr; }
                        SDL_Surface* surf = IMG_Load(opts[choice].c_str());
                        if (surf) {
                            bgTexture = SDL_CreateTextureFromSurface(ren, surf);
                            SDL_FreeSurface(surf);
                        }
                    }
                }


                if (inR(mx, my, btnClearBg)) {
                    if (bgTexture) { SDL_DestroyTexture(bgTexture); bgTexture = nullptr; }
                }


                if (mx < PALETTE_WIDTH && my >= TOOLBAR_H && my < SH - PANEL_HEIGHT) {
                    for (int i = 0; i < (int)palette.size(); i++) {
                        SDL_Rect adj = palette[i].rect; adj.y -= palScrollY;
                        if (inR(mx, my, adj)) {
                            draggingBlock = true;
                            draggedBlock = palette[i];
                            draggedBlock.rect.x = mx; draggedBlock.rect.y = my;
                            blkOX = mx - palette[i].rect.x; blkOY = my - adj.y;
                            break;
                        }
                    }
                }


                if (ev.button.clicks == 2 && mx >= PALETTE_WIDTH && mx < PALETTE_WIDTH + scriptPanelW
                    && my >= TOOLBAR_H && my < SH - PANEL_HEIGHT && !playing) {
                    for (int i = 0; i < (int)userBlocks.size(); i++) {
                        SDL_Rect adj = userBlocks[i].rect; adj.y -= scriptScrollY;
                        if (inR(mx, my, adj)) {
                            EditBlock(userBlocks[i], ren, font, SW, SH);
                            Engine_LoadBlocks(engine, userBlocks);
                            break;
                        }
                    }
                }


                if (ev.button.button == SDL_BUTTON_RIGHT && mx >= PALETTE_WIDTH && mx < PALETTE_WIDTH + scriptPanelW
                    && my >= TOOLBAR_H && my < SH - PANEL_HEIGHT && !playing) {
                    for (int i = 0; i < (int)userBlocks.size(); i++) {
                        SDL_Rect adj = userBlocks[i].rect; adj.y -= scriptScrollY;
                        if (inR(mx, my, adj)) {
                            userBlocks.erase(userBlocks.begin() + i);

                            for (int j = i; j < (int)userBlocks.size(); j++)
                                userBlocks[j].rect.y = TOOLBAR_H + 8 + j * (BLOCK_H + 6);
                            Engine_LoadBlocks(engine, userBlocks);
                            undoPush();
                            break;
                        }
                    }
                }


                if (mx >= sceneX && mx < sceneX + sceneW && my >= sceneY && my < sceneY + sceneH) {
                    for (int i = 0; i < (int)sprites.size(); i++) {
                        if (Sprite_ContainsPoint(sprites[i], mx, my)) {
                            draggedSprite = &sprites[i];
                            sprites[i].beingDragged = true;
                            sprites[i].dragOffX = mx - (int)sprites[i].x;
                            sprites[i].dragOffY = my - (int)sprites[i].y;
                            activeIndex = i;
                            break;
                        }
                    }
                }
            }

            if (ev.type == SDL_MOUSEBUTTONUP) {
                if (draggingBlock) {
                    int mx = ev.button.x, my = ev.button.y;
                    if (mx >= PALETTE_WIDTH && mx < PALETTE_WIDTH + scriptPanelW
                        && my >= TOOLBAR_H && my < SH - PANEL_HEIGHT) {
                        int yy = TOOLBAR_H + 8 + (int)userBlocks.size() * (BLOCK_H + 6);
                        UIBlock nb = draggedBlock;
                        nb.rect = {PALETTE_WIDTH + 8, yy, scriptBlockW, BLOCK_H};
                        userBlocks.push_back(nb);
                        Engine_LoadBlocks(engine, userBlocks);
                        undoPush();
                    }
                    draggingBlock = false;
                }
                if (draggedSprite) { draggedSprite->beingDragged = false; draggedSprite = nullptr; }
            }

            if (ev.type == SDL_MOUSEMOTION) {
                if (draggingBlock) {
                    draggedBlock.rect.x = ev.motion.x - blkOX;
                    draggedBlock.rect.y = ev.motion.y - blkOY;
                }
                if (draggedSprite) {
                    draggedSprite->x = ev.motion.x - draggedSprite->dragOffX;
                    draggedSprite->y = ev.motion.y - draggedSprite->dragOffY;
                    Sprite_ClampToScene(*draggedSprite, sceneX, sceneY, sceneW, sceneH);
                }
            }
        }

        for (auto& s : sprites) Sprite_UpdateBubble(s, dt);


        {
            int mx, my;
            SDL_GetMouseState(&mx, &my);
            ctx.mouseX = mx;
            ctx.mouseY = my;
        }

        if (playing) {
            engine.stepCount = 0;


            if (engine.isWaiting && engine.waitTimer == -999.f && !sprites.empty()) {
                string question = ctx.lastAnswer;
                string answer;
                ShowInputDialog(ren, font, SW, SH, question, "", answer);
                ctx.lastAnswer = answer;
                if (ctx.vars) (*ctx.vars)["answer"] = 0.0;

                sprites[activeIndex].bubble.text     = answer;
                sprites[activeIndex].bubble.active   = !answer.empty();
                sprites[activeIndex].bubble.isThink  = false;
                sprites[activeIndex].bubble.timeLeft = 3.f;
                engine.isWaiting = false;
                engine.waitTimer = 0.f;
                engine.pc++;
            }

            if (!sprites.empty() && !Engine_RunStep(engine, sprites[activeIndex], ctx, dt))
                playing = false;
            else if (sprites.empty())
                playing = false;
        }


        SDL_SetRenderDrawColor(ren, 30, 30, 45, 255);
        SDL_RenderClear(ren);


        SDL_Rect sceneRect = {sceneX, sceneY, sceneW, sceneH};
        SDL_SetRenderDrawColor(ren, 245, 245, 250, 255);
        SDL_RenderFillRect(ren, &sceneRect);
        SDL_RenderSetClipRect(ren, &sceneRect);

        if (bgTexture)
            SDL_RenderCopy(ren, bgTexture, nullptr, &sceneRect);

        if (penLayerTex)
            SDL_RenderCopy(ren, penLayerTex, nullptr, &sceneRect);
        for (auto& s : sprites) Sprite_Render(s, ren, font);
        SDL_RenderSetClipRect(ren, nullptr);
        SDL_SetRenderDrawColor(ren, 90, 90, 130, 255);
        SDL_RenderDrawRect(ren, &sceneRect);


        SDL_Rect panelRect = {sceneX, sceneY + sceneH, sceneW, PANEL_HEIGHT};
        SDL_SetRenderDrawColor(ren, 28, 28, 44, 255);
        SDL_RenderFillRect(ren, &panelRect);
        if (!sprites.empty() && font) {
            const Sprite& cs = sprites[activeIndex];
            string info = "x:" + to_string((int)(cs.x - sceneX)) +
                          "  y:" + to_string((int)(cs.y - sceneY)) +
                          "  dir:" + to_string((int)cs.direction) + "°" +
                          "  size:" + to_string((int)cs.sizePct) + "%" +
                          (playing ? "  [RUNNING]" : "");
            SDL_Rect ir = {panelRect.x + 10, panelRect.y + 8, panelRect.w - 20, 28};
            DrawText(ren, font, info, white, ir, true);


            int vx = panelRect.x + 10, vy = panelRect.y + 44;
            for (auto& kv : varStore) {
                string vs = kv.first + " = " + to_string(kv.second).substr(0, 6);
                SDL_Rect vr = {vx, vy, 150, 24};
                SDL_SetRenderDrawColor(ren, 50, 50, 80, 200);
                SDL_RenderFillRect(ren, &vr);
                SDL_Color yell = {255,220,80,255};
                DrawText(ren, font, vs, yell, vr, true);
                vx += 160;
            }
        }


        SDL_Rect palRect = {0, TOOLBAR_H, PALETTE_WIDTH, SH - TOOLBAR_H - PANEL_HEIGHT};
        SDL_SetRenderDrawColor(ren, 38, 38, 62, 255);
        SDL_RenderFillRect(ren, &palRect);
        SDL_RenderSetClipRect(ren, &palRect);
        for (auto& ub : palette) {
            SDL_Rect adj = ub.rect; adj.y -= palScrollY;
            if (adj.y + adj.h < TOOLBAR_H || adj.y > SH - PANEL_HEIGHT) continue;
            SDL_Color bc = ub.color; bc.a = 255;
            DrawBlockAuto(ren, adj, bc, ub.blockData);
            DrawText(ren, font, ub.label, white, adj);
        }
        SDL_RenderSetClipRect(ren, nullptr);


        SDL_Rect scrRect = {PALETTE_WIDTH, TOOLBAR_H, scriptPanelW, SH - TOOLBAR_H - PANEL_HEIGHT};
        SDL_SetRenderDrawColor(ren, 48, 48, 72, 255);
        SDL_RenderFillRect(ren, &scrRect);


        if (userBlocks.empty() && font) {
            SDL_Color grey = {120,120,160,255};
            SDL_Rect hr = {PALETTE_WIDTH+10, TOOLBAR_H+20, scriptPanelW-20, 30};
            DrawText(ren, font, "Drag blocks here", grey, hr);
            SDL_Rect hr2 = {PALETTE_WIDTH+10, TOOLBAR_H+50, scriptPanelW-20, 25};
            DrawText(ren, font, "Dbl-click to edit", grey, hr2);
            SDL_Rect hr3 = {PALETTE_WIDTH+10, TOOLBAR_H+75, scriptPanelW-20, 25};
            DrawText(ren, font, "Right-click to delete", grey, hr3);
        }

        SDL_RenderSetClipRect(ren, &scrRect);
        for (int i = 0; i < (int)userBlocks.size(); i++) {
            SDL_Rect adj = userBlocks[i].rect; adj.y -= scriptScrollY;
            if (adj.y + adj.h < TOOLBAR_H || adj.y > SH - PANEL_HEIGHT) continue;
            bool act = playing && i == engine.pc;
            SDL_Color bc = act ? SDL_Color{255, 230, 0, 255} : userBlocks[i].color;
            bc.a = 255;
            DrawBlockAuto(ren, adj, bc, userBlocks[i].blockData);

            SDL_Rect lblRect = adj;
            if (userBlocks[i].blockData.isContainer) { lblRect.x += 8; lblRect.w -= 8; }
            DrawText(ren, font, userBlocks[i].label, act ? black : white, lblRect);
        }
        SDL_RenderSetClipRect(ren, nullptr);


        if (draggingBlock) {
            SDL_Color dc = draggedBlock.color; dc.a = 200;
            DrawBlockAuto(ren, draggedBlock.rect, dc, draggedBlock.blockData);
            DrawText(ren, font, draggedBlock.label, white, draggedBlock.rect);
        }


        SDL_SetRenderDrawColor(ren, 20, 20, 35, 255);
        SDL_Rect tb = {0, 0, SW, TOOLBAR_H};
        SDL_RenderFillRect(ren, &tb);
        DrawButton(ren, font, btnPlay,         playing ? SDL_Color{20,160,20,255} : SDL_Color{40,180,40,255}, playing ? "> Playing" : "> Play");
        DrawButton(ren, font, btnStop,         {180,40,40,255},   "[ ] Stop");
        DrawButton(ren, font, btnSave,         {40,100,180,255},  "Save");
        DrawButton(ren, font, btnLoad,         {100,60,180,255},  "Load");
        DrawButton(ren, font, btnClear,        {150,70,20,255},   "Clear");
        DrawButton(ren, font, btnUndo,         undoPos > 0 ? SDL_Color{80,80,140,255} : SDL_Color{50,50,80,255}, "Undo");
        DrawButton(ren, font, btnRedo,         undoPos < (int)undoStack.size()-1 ? SDL_Color{80,80,140,255} : SDL_Color{50,50,80,255}, "Redo");
        DrawButton(ren, font, btnAddSprite,    {40,140,100,255},  "+ Sprite");
        DrawButton(ren, font, btnDeleteSprite, sprites.empty() ? SDL_Color{60,30,30,255} : SDL_Color{180,50,50,255}, "Del Sprite");
        DrawButton(ren, font, btnChangeBg,     {100,60,160,255},  "Set BG");
        DrawButton(ren, font, btnClearBg,      bgTexture ? SDL_Color{120,70,30,255} : SDL_Color{60,50,30,255}, "Clear BG");

        if (playing)
            DrawButton(ren, font, btnPause, engine.paused ? SDL_Color{200,180,0,255} : SDL_Color{80,80,110,255},
                       engine.paused ? ">> Resume" : "|| Pause");
        DrawButton(ren, font, btnQuit,         {160,30,30,255},   "X Quit");


        if (font) {
            string timerStr = "T:" + to_string((int)ctx.timer) + "s";
            SDL_Rect tr = {SW - 190, 10, 100, 30};
            DrawText(ren, font, timerStr, {180,220,255,255}, tr);
            if (!userBlocks.empty()) {
                string bc = to_string(userBlocks.size()) + " blocks";
                SDL_Rect br = {PALETTE_WIDTH + 4, TOOLBAR_H - 18, scriptPanelW - 8, 16};
                SDL_Color gc = {120,120,160,255};
                DrawText(ren, font, bc, gc, br);
            }
        }

        SDL_RenderPresent(ren);
    }

    if (defTex.tex)   SDL_DestroyTexture(defTex.tex);
    if (bgTexture)    SDL_DestroyTexture(bgTexture);
    if (penLayerTex)  SDL_DestroyTexture(penLayerTex);
    if (font) TTF_CloseFont(font);
    Sound_Quit();
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    TTF_Quit(); IMG_Quit(); SDL_Quit();
    return 0;
}