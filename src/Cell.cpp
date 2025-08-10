#include "Cell.hpp"
#include "Powerup.hpp"

void Cell::draw(SDL_Renderer* r, const SDL_FRect& rc) const{
    SDL_SetRenderDrawColor(r, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(r, &rc);
    if(power) power->draw(r, rc);
}

void Cell::activate(Engine* e, size_t i){ if(power){ power->apply(e,i); power.reset(); } }


