

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
