#pragma once
#include "Engine.hpp"

class Powerup {
public:
    virtual ~Powerup() = default;
    virtual void draw(SDL_Renderer* r, const SDL_FRect& rc) const = 0;
    virtual void apply(class Engine* e, size_t i) = 0;
};

class Paint3 : public Powerup {
public:
    void draw(SDL_Renderer* r, const SDL_FRect& rc) const override;
    void apply(Engine* e, size_t i) override;
};

class Bomb5 : public Powerup {
public:
    void draw(SDL_Renderer* r, const SDL_FRect& rc) const override;
    void apply(Engine* e, size_t i) override;
};


