#pragma once
#include "SDL3/SDL.h"
#include <memory>
#include <vector>

constexpr int W = 800;
constexpr int H = 600;
constexpr int CELL_W = 40;
constexpr int CELL_H = 40;
constexpr int COLS = W / CELL_W;
constexpr int ROWS = H / CELL_H;
constexpr int COUNT = COLS * ROWS;

struct Color { Uint8 r,g,b,a; };
extern const Color COLORS[];
extern const int COLORS_N;

struct Drawable {
    virtual ~Drawable() = default;
    virtual void draw(SDL_Renderer* r, const SDL_FRect& rc) const = 0;
};

class Cell;

class Engine {
public:
    Engine();
    bool init();
    void run();

    // простые методы для бонусов
    std::shared_ptr<Cell> cell(size_t i);
    void setColor(size_t i, Color c);
    void dropAt(size_t i, int times = 1);
    Color randColor() const;

private:
    SDL_Window* win{nullptr};
    SDL_Renderer* ren{nullptr};
    bool quit{false};
    int selected{-1};

    std::vector<std::shared_ptr<Drawable>> grid;

    void initGrid();
    void process();
    void update();
    void render() const;
    void click(const SDL_MouseButtonEvent& e);
    bool matches();
    bool canPlace(size_t i, Color c) const;
    void dropCol(int col, int fromRow, int removed);
    void spawnPower(size_t origin);
};


