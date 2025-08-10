#pragma once

#include <SFML/Graphics.hpp>

enum class BonusType {
  ExpandPaddle,
  CompressPaddle,
  SlowBall,
  FastBall,
  Sticky,
  OneTimeFloor,
  RandomBounce,
  SpawnMovingBlock,
  SpawnSecondBall
};

struct Bonus {
  BonusType type{};
  sf::Vector2f pos{};
  float speed = 120.f;
  bool taken = false;
};


