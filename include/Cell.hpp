#pragma once
#include "Engine.hpp"
#include <memory>

class Powerup;

class Cell : public Drawable {
public:
    explicit Cell(Color c) : color(c) {}
    void draw(SDL_Renderer* r, const SDL_FRect& rc) const override;

    void setPower(std::unique_ptr<Powerup> p) { power = std::move(p); }
    bool hasPower() const { return static_cast<bool>(power); }
    void activate(Engine* e, size_t i);

    Color color{255,255,255,255};
private:
    std::unique_ptr<Powerup> power;
};


