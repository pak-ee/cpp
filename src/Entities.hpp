#pragma once

#include <SFML/Graphics.hpp>

struct Paddle {
  sf::Vector2f pos{400.f, 560.f};
  float width = 100.f;
  float height = 16.f;
  float speed = 420.f;
  bool sticky = false;
};

struct Ball {
  sf::Vector2f pos{400.f, 520.f};
  sf::Vector2f vel{220.f, -260.f};
  float radius = 8.f;
  bool attached = true;
  float paddleCollisionCooldownSec = 0.f;
  float stickyIgnoreSec = 0.f; // ignore sticky effect for some time
};

struct Block {
  sf::FloatRect rect{};
  int hp = 1;                 // -1 = indestructible
  int onHitSpeedDelta = 0;    // +1 speed up
  bool hasBonus = false;      // drop bonus on destroy
  bool moving = false;        // moving block bonus
  sf::Vector2f moveVel{80.f, 0.f};
  int hitsLeft = 3;           // for moving block lifespan
};


