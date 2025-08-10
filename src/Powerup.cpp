#include "Powerup.hpp"
#include "Cell.hpp"
#include <random>
#include <algorithm>

static int rndp(int a, int b){ static std::random_device rd; static std::mt19937 g(rd()); std::uniform_int_distribution<int> d(a,b-1); return d(g);} 
static bool neigh(size_t a, size_t b){ if(a==b) return false; int ax=a%COLS,ay=a/COLS,bx=b%COLS,by=b/COLS; return (ax==bx&&std::abs(ay-by)==1)||(ay==by&&std::abs(ax-bx)==1);} 

void Paint3::draw(SDL_Renderer* r, const SDL_FRect& rc) const{ SDL_SetRenderDrawColor(r, 40, 140, 255, 255); SDL_FRect b{ rc.x+rc.w*0.3f, rc.y+rc.h*0.3f, rc.w*0.4f, rc.h*0.4f }; SDL_RenderFillRect(r, &b);} 

void Paint3::apply(Engine* e, size_t i){
    auto c = e->cell(i); if(!c) return; Color src = c->color; c->color = e->randColor();
    int x=i%COLS, y=i/COLS;
    auto pick=[&](int r){ int px = std::max(0, std::min(COLS-1, rndp(std::max(0,x-r), std::min(COLS,x+r)))); int py = std::max(0, std::min(ROWS-1, rndp(std::max(0,y-r), std::min(ROWS,y+r)))); return size_t(px+py*COLS); };
    size_t a = pick(2); while(neigh(i,a)) a = pick(2); if(auto t=e->cell(a)) t->color = src;
    size_t b = pick(2); while(neigh(i,b) || b==a) b = pick(2); if(auto t=e->cell(b)) t->color = src;
}

void Bomb5::draw(SDL_Renderer* r, const SDL_FRect& rc) const{ SDL_SetRenderDrawColor(r, 255, 60, 60, 255); SDL_FRect b{ rc.x+rc.w*0.35f, rc.y+rc.h*0.35f, rc.w*0.3f, rc.h*0.3f }; SDL_RenderFillRect(r, &b);} 

void Bomb5::apply(Engine* e, size_t i){ e->dropAt(i,1); for(int k=0;k<4;++k){ size_t j = rndp(0,COLS) + rndp(0,ROWS)*COLS; e->dropAt(j,1);} }


