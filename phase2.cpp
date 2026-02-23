

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