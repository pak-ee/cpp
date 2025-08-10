#include "Engine.hpp"
#include "Cell.hpp"
#include "Powerup.hpp"
#include <random>
#include <algorithm>

static int rnd(int a, int b) {
    static std::random_device rd; static std::mt19937 g(rd());
    std::uniform_int_distribution<int> d(a, b - 1);
    return d(g);
}

const Color COLORS[] = {
    {200, 70, 70, 255}, {70, 200, 120, 255}, {70, 120, 200, 255},
    {220, 180, 60, 255}, {150, 90, 160, 255}, {90, 90, 90, 255}
};
const int COLORS_N = sizeof(COLORS) / sizeof(COLORS[0]);

static bool same(const Color& a, const Color& b){return a.r==b.r&&a.g==b.g&&a.b==b.b&&a.a==b.a;}
static bool neigh(size_t a, size_t b){ if(a==b) return false; int ax=a%COLS,ay=a/COLS,bx=b%COLS,by=b/COLS; return (ax==bx&&std::abs(ay-by)==1)||(ay==by&&std::abs(ax-bx)==1);} 

Engine::Engine(){ grid.reserve(COUNT); }

bool Engine::init(){
    if(!SDL_Init(SDL_INIT_VIDEO)){ SDL_Log("SDL init failed: %s", SDL_GetError()); return false; }
    win = SDL_CreateWindow("Gems Alt", W, H, SDL_WINDOW_RESIZABLE);
    if(!win) return false;
    ren = SDL_CreateRenderer(win, nullptr);
    if(!ren) return false;
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
    initGrid();
    return true;
}

void Engine::initGrid(){
    for(int i=0;i<COUNT;++i){
        Color c = COLORS[rnd(0, COLORS_N)];
        while(!canPlace(i,c)) c = COLORS[rnd(0, COLORS_N)];
        grid.push_back(std::make_shared<Cell>(c));
    }
}

bool Engine::canPlace(size_t i, Color c) const{
    if(i >= 2*COLS){
        auto a = std::dynamic_pointer_cast<Cell>(grid[i-COLS]);
        auto b = std::dynamic_pointer_cast<Cell>(grid[i-2*COLS]);
        if(a&&b&&same(a->color,c)&&same(b->color,c)) return false;
    }
    if(i % COLS >= 2){
        auto a = std::dynamic_pointer_cast<Cell>(grid[i-1]);
        auto b = std::dynamic_pointer_cast<Cell>(grid[i-2]);
        if(a&&b&&same(a->color,c)&&same(b->color,c)) return false;
    }
    return true;
}

void Engine::run(){
    while(!quit){ process(); update(); render(); SDL_Delay(100); }
}

void Engine::process(){ SDL_Event e; while(SDL_PollEvent(&e)){ if(e.type==SDL_EVENT_QUIT) quit=true; else if(e.type==SDL_EVENT_MOUSE_BUTTON_DOWN) click(e.button);} }

void Engine::click(const SDL_MouseButtonEvent& e){
    if(e.button!=SDL_BUTTON_LEFT || !e.down){ selected=-1; return; }
    int x = e.x / CELL_W, y = e.y / CELL_H; size_t idx = x + y*COLS;
    if(selected!=-1 && neigh(selected, idx)){ std::swap(grid[selected], grid[idx]); selected=-1; }
    else selected = (int)idx;
}

void Engine::render() const{
    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255); SDL_RenderClear(ren);
    for(int i=0;i<(int)grid.size();++i){ int gx=i%COLS, gy=i/COLS; SDL_FRect r{float(gx*CELL_W), float(gy*CELL_H), float(CELL_W), float(CELL_H)}; grid[i]->draw(ren, r);}    
    if(selected!=-1 && selected<(int)grid.size()){ int gx=selected%COLS, gy=selected/COLS; SDL_FRect r{float(gx*CELL_W), float(gy*CELL_H), float(CELL_W), float(CELL_H)}; SDL_SetRenderDrawColor(ren,255,255,255,96); SDL_RenderFillRect(ren,&r); SDL_SetRenderDrawColor(ren,0,0,0,255); SDL_RenderRect(ren,&r);}    
    SDL_RenderPresent(ren);
}

bool Engine::matches(){
    for(int i=0;i<COUNT;++i){ auto pivot = std::dynamic_pointer_cast<Cell>(grid[i]); if(!pivot) continue; 
        // vertical
        if(i>=2*COLS){ auto a=std::dynamic_pointer_cast<Cell>(grid[i-COLS]); auto b=std::dynamic_pointer_cast<Cell>(grid[i-2*COLS]); if(a&&b&&same(a->color,pivot->color)&&same(b->color,pivot->color)){
            int extra=0; while(i+(extra+1)*COLS<COUNT){ auto n=std::dynamic_pointer_cast<Cell>(grid[i+(extra+1)*COLS]); if(n&&same(n->color,pivot->color)) extra++; else break;}
            int col=i%COLS; int startRow=i/COLS-2; int endRow=startRow+2+extra; for(int r=startRow;r<=endRow;++r) spawnPower(col+r*COLS);
            dropCol(col,endRow,3+extra); return true; }
        }
        // horizontal
        if(i%COLS>=2){ auto a=std::dynamic_pointer_cast<Cell>(grid[i-1]); auto b=std::dynamic_pointer_cast<Cell>(grid[i-2]); if(a&&b&&same(a->color,pivot->color)&&same(b->color,pivot->color)){
            int row=i/COLS; int end=(row+1)*COLS; int extra=0; while(i+extra+1<end){ auto n=std::dynamic_pointer_cast<Cell>(grid[i+extra+1]); if(n&&same(n->color,pivot->color)) extra++; else break;}
            int startCol=i%COLS-2; int endCol=startCol+2+extra; for(int c=startCol;c<=endCol;++c) spawnPower(c+row*COLS);
            for(int c=startCol;c<=endCol;++c) dropCol(c,row,1); return true; }
        }
    }
    return false;
}

void Engine::dropCol(int col, int fromRow, int removed){
    for(int r=fromRow; r>=removed; --r){ int dst = col + r*COLS; int src = col + (r-removed)*COLS; grid[dst] = grid[src]; }
    for(int r=0;r<removed;++r){ int idx = col + r*COLS; grid[idx] = std::make_shared<Cell>(COLORS[rnd(0,COLORS_N)]); }
}

void Engine::update(){
    // активируем бонусы и затем ищем совпадения
    for(size_t i=0;i<grid.size();++i){ if(auto c=std::dynamic_pointer_cast<Cell>(grid[i]); c && c->hasPower()) c->activate(this,i); }
    matches();
}

void Engine::spawnPower(size_t origin){
    if(rnd(0,100) > 50) return; // 50%
    int x=origin%COLS, y=origin/COLS;
    int rx=std::max(0,std::min(COLS-1, rnd(std::max(0,x-3), std::min(COLS,x+3))));
    int ry=std::max(0,std::min(ROWS-1, rnd(std::max(0,y-3), std::min(ROWS,y+3))));
    size_t idx = rx + ry*COLS;
    if(auto c = std::dynamic_pointer_cast<Cell>(grid[idx])){
        if(rnd(0,2)==0) c->setPower(std::make_unique<Paint3>());
        else c->setPower(std::make_unique<Bomb5>());
    }
}

std::shared_ptr<Cell> Engine::cell(size_t i){ if(i>=grid.size()) return nullptr; return std::dynamic_pointer_cast<Cell>(grid[i]); }
void Engine::setColor(size_t i, Color c){ if(auto t=cell(i)) t->color=c; }
void Engine::dropAt(size_t i, int times){ if(i>=grid.size()||times<=0) return; int col=i%COLS, row=i/COLS; dropCol(col,row,times); }
Color Engine::randColor() const { return COLORS[rnd(0,COLORS_N)]; }


