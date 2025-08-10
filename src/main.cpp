#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <cmath>
#include <optional>
#include <random>
#include <vector>
#include "Entities.hpp"
#include "Bonus.hpp"
#include "Physics.hpp"

static std::mt19937& rng() {
  static thread_local std::mt19937 gen{std::random_device{}()};
  return gen;
}

static void enforceAngle(sf::Vector2f& v, float maxXRatio = 0.8f, float minYRatio = 0.6f) {
  float speed = std::sqrt(v.x * v.x + v.y * v.y);
  if (speed < 1e-3f) { v = {0.f, -200.f}; return; }
  float maxX = maxXRatio * speed;
  if (std::abs(v.x) > maxX) v.x = (v.x > 0.f ? maxX : -maxX);
  float minY = minYRatio * speed;
  if (std::abs(v.y) < minY) v.y = (v.y >= 0.f ? minY : -minY);
}

void applyBonus(Paddle& paddle,
                std::vector<Ball>& balls,
                std::vector<Block>& blocks,
                std::optional<bool>& oneTimeFloor,
                BonusType type,
                const sf::Vector2u& winSize,
                bool& movingBlockSpawned) {
  switch (type) {
    case BonusType::ExpandPaddle: paddle.width = std::min(200.f, paddle.width + 40.f); break;
    case BonusType::CompressPaddle: paddle.width = std::max(60.f, paddle.width - 30.f); break;
    case BonusType::SlowBall: for (auto& b : balls) b.vel *= 0.8f; break;
    case BonusType::FastBall: for (auto& b : balls) b.vel *= 1.25f; break;
    case BonusType::Sticky: paddle.sticky = true; break;
    case BonusType::OneTimeFloor: oneTimeFloor = true; break;
    case BonusType::RandomBounce: {
      std::uniform_real_distribution<float> angleDist(-0.6f, 0.6f);
      for (auto& b : balls) {
        float angle = angleDist(rng());
        float speed = std::sqrt(b.vel.x * b.vel.x + b.vel.y * b.vel.y);
        b.vel = sf::Vector2f{std::sin(angle) * speed, -std::cos(angle) * speed};
        enforceAngle(b.vel);
      }
    } break;
    case BonusType::SpawnMovingBlock: {
      if (!movingBlockSpawned) {
        // Порождаем подвижный блок вверху, едущий горизонтально
        Block nb; nb.rect = {20.f, 140.f, 60.f, 20.f}; nb.moving = true; nb.hp = 3; nb.hitsLeft = 3; nb.hasBonus = false; nb.onHitSpeedDelta = 0;
        // Не задеваем существующие блоки при спавне
        bool ok = true; for (auto& b : blocks) if (b.hp != 0 && nb.rect.intersects(b.rect)) { ok = false; break; }
        if (ok) { blocks.push_back(nb); movingBlockSpawned = true; }
        else {
          // если не удалось — вместо этого даём второй мяч
          type = BonusType::SpawnSecondBall;
          // fallthrough to SpawnSecondBall
        }
      } else {
        // уже был — заменяем на второй мяч
        type = BonusType::SpawnSecondBall;
      }
      if (type != BonusType::SpawnMovingBlock) {
        // обработать замену на второй мяч
        if (balls.size() < 2) {
          Ball nb = balls.front();
          nb.pos += sf::Vector2f{12.f, -6.f};
          float speed = std::sqrt(nb.vel.x * nb.vel.x + nb.vel.y * nb.vel.y);
          float angle = std::atan2(nb.vel.y, nb.vel.x) + 0.35f;
          nb.vel = sf::Vector2f{std::cos(angle) * speed, std::sin(angle) * speed};
          enforceAngle(nb.vel);
          nb.attached = false;
          nb.paddleCollisionCooldownSec = 0.15f;
          nb.stickyIgnoreSec = 0.40f;
          balls.push_back(nb);
        }
      }
    } break;
    case BonusType::SpawnSecondBall: {
      if (balls.size() < 2) {
        Ball nb = balls.front();
        nb.pos += sf::Vector2f{12.f, -6.f}; // небольшой вертикальный оффсет
        // rotate velocity slightly instead of pure mirror to avoid following
        float speed = std::sqrt(nb.vel.x * nb.vel.x + nb.vel.y * nb.vel.y);
        float angle = std::atan2(nb.vel.y, nb.vel.x) + 0.35f; // +20 degrees
        nb.vel = sf::Vector2f{std::cos(angle) * speed, std::sin(angle) * speed};
        enforceAngle(nb.vel);
        nb.attached = false;
        nb.paddleCollisionCooldownSec = 0.15f; // исключить мгновенный повторный контакт
        nb.stickyIgnoreSec = 0.40f;            // на время игнорировать липкость платформы
        balls.push_back(nb);
      }
    } break;
  }
}

int main() {
  const unsigned windowW = 800;
  const unsigned windowH = 600;
  sf::RenderWindow window(sf::VideoMode(windowW, windowH), "Minimal Arkanoid");
  window.setFramerateLimit(120);

  // Level setup
  std::vector<Block> blocks;
  const int rows = 3;
  const int cols = 8;
  const float margin = 8.f;
  const float bw = (windowW - margin * (cols + 1)) / cols;
  const float bh = 20.f;
  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < cols; ++c) {
      Block b;
      b.rect = {margin + c * (bw + margin), 80.f + r * (bh + margin), bw, bh};
      if (r == 0 && (c % 4 == 0)) { b.hp = -1; }            // indestructible
      else if (r == 1) { b.hp = 2; }                         // multi-HP row
      else { b.hp = 1; }
      if (r == 1 && (c == cols / 2 || c == cols / 2 - 1)) { b.onHitSpeedDelta = +1; } // speed-up in center
      if (r == 2 && (c % 2 == 0)) { b.hasBonus = true; }
      blocks.push_back(b);
    }
  }

  Paddle paddle;
  std::vector<Ball> balls(1);
  balls[0] = Ball{};
  balls[0].vel = {160.f, -200.f};
  int score = 0;
  int lives = 3;
  std::optional<bool> oneTimeFloor; // present if true
  bool movingBlockSpawned = false;
  std::vector<Bonus> bonuses;

  sf::Clock clock;
  bool leftHeld = false, rightHeld = false;

  while (window.isOpen()) {
    sf::Event ev;
    while (window.pollEvent(ev)) {
      if (ev.type == sf::Event::Closed) window.close();
      if (ev.type == sf::Event::KeyPressed) {
        if (ev.key.code == sf::Keyboard::Escape) window.close();
        if (ev.key.code == sf::Keyboard::Left) leftHeld = true;
        if (ev.key.code == sf::Keyboard::Right) rightHeld = true;
        if (ev.key.code == sf::Keyboard::Space) {
          if (!balls.empty() && balls[0].attached) {
            balls[0].attached = false;
            if (paddle.sticky) paddle.sticky = false; // launch clears sticky hold
          }
        }
        if (ev.key.code == sf::Keyboard::R) {
          // Random bounce bonus trigger
          applyBonus(paddle, balls, blocks, oneTimeFloor, BonusType::RandomBounce, window.getSize(), movingBlockSpawned);
        }
      }
      if (ev.type == sf::Event::KeyReleased) {
        if (ev.key.code == sf::Keyboard::Left) leftHeld = false;
        if (ev.key.code == sf::Keyboard::Right) rightHeld = false;
      }
    }

    float dt = clock.restart().asSeconds();
    dt = std::min(dt, 0.02f);

    // Update paddle
    float dir = (rightHeld ? 1.f : 0.f) - (leftHeld ? 1.f : 0.f);
    paddle.pos.x += dir * paddle.speed * dt;
    paddle.pos.x = clamp(paddle.pos.x, paddle.width * 0.5f, windowW - paddle.width * 0.5f);

    // Balls update
    for (auto& ball : balls) {
      if (ball.paddleCollisionCooldownSec > 0.f) ball.paddleCollisionCooldownSec -= dt;
      if (ball.attached) {
        ball.pos.x = paddle.pos.x;
        ball.pos.y = paddle.pos.y - paddle.height * 0.5f - ball.radius - 1.f;
      } else {
        ball.pos += ball.vel * dt;
      }
      if (ball.stickyIgnoreSec > 0.f) ball.stickyIgnoreSec -= dt;
    }

    // Wall collisions
    for (auto& ball : balls) {
      if (ball.pos.x - ball.radius < 0.f) { ball.pos.x = ball.radius; ball.vel = reflectHorizontal(ball.vel); }
      if (ball.pos.x + ball.radius > windowW) { ball.pos.x = windowW - ball.radius; ball.vel = reflectHorizontal(ball.vel); }
      if (ball.pos.y - ball.radius < 0.f) { ball.pos.y = ball.radius; ball.vel = reflectVertical(ball.vel); }
    }

    // Bottom
    for (auto& ball : balls) {
      if (!(ball.pos.y - ball.radius > windowH)) continue;
      if (oneTimeFloor && *oneTimeFloor) {
        // bounce as from paddle and consume floor
        ball.pos.y = windowH - ball.radius - 1.f;
        ball.vel = reflectVertical(ball.vel);
        oneTimeFloor.reset();
      } else {
        lives -= 1;
        ball.attached = true;
        ball.pos = {paddle.pos.x, paddle.pos.y - paddle.height * 0.5f - ball.radius - 1.f};
        ball.vel = {220.f, -260.f};
        if (lives < 0) {
          // Simple reset on game over
          lives = 3; score = 0; oneTimeFloor.reset();
          blocks.clear(); bonuses.clear(); balls.resize(1); balls[0] = Ball{}; balls[0].attached = true;
          for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
              Block b;
              b.rect = {margin + c * (bw + margin), 60.f + r * (bh + margin), bw, bh};
              if (r == 0 && (c % 3 == 0)) { b.hp = -1; }
              else if (r % 2 == 0) { b.hp = 2; }
              if (r == 1 && (c % 4 == 0)) { b.onHitSpeedDelta = +1; }
              if (r == 2 && (c % 3 == 1)) { b.hasBonus = true; }
              blocks.push_back(b);
            }
          }
        }
      }
    }

    // Paddle collision
    sf::FloatRect paddleRect(
      paddle.pos.x - paddle.width * 0.5f,
      paddle.pos.y - paddle.height * 0.5f,
      paddle.width,
      paddle.height
    );
    for (auto& ball : balls) if (!ball.attached) {
      sf::Vector2f normal;
      if (circleRectIntersect(paddleRect, ball.pos, ball.radius, &normal) && ball.paddleCollisionCooldownSec <= 0.f) {
        // Reflect and apply angle based on hit point
        ball.vel = reflectVertical(ball.vel);
        float t = (ball.pos.x - paddle.pos.x) / (paddle.width * 0.5f);
        t = clamp(t, -0.7f, 0.7f);
        float speed = std::sqrt(ball.vel.x * ball.vel.x + ball.vel.y * ball.vel.y);
        ball.vel.x = t * speed;
        ball.vel.y = -std::abs(ball.vel.y);
        enforceAngle(ball.vel);
        // push ball slightly above paddle to avoid sticking and set cooldown
        ball.pos.y = paddleRect.top - ball.radius - 0.5f;
        ball.paddleCollisionCooldownSec = 0.08f;
        if (paddle.sticky && ball.stickyIgnoreSec <= 0.f) { ball.attached = true; }
      }
    }

    // Block collisions
    for (auto& b : blocks) {
      if (b.hp == 0) continue;
      // Moving block update (bonus 8)
      if (b.moving && b.hp > 0) {
        b.rect.left += b.moveVel.x * dt;
        if (b.rect.left < 0.f || b.rect.left + b.rect.width > windowW) b.moveVel.x = -b.moveVel.x;
      }
      for (auto& ball : balls) {
        sf::Vector2f normal;
        if (circleRectIntersect(b.rect, ball.pos, ball.radius, &normal)) {
          if (std::abs(normal.x) > 0.f) ball.vel = reflectHorizontal(ball.vel);
          if (std::abs(normal.y) > 0.f) ball.vel = reflectVertical(ball.vel);
          if (b.onHitSpeedDelta > 0) ball.vel *= 1.10f;
          enforceAngle(ball.vel);
          if (b.hp > 0) {
            b.hp -= 1; score += 1;
            if (b.moving) { b.hitsLeft -= 1; if (b.hitsLeft <= 0) b.hp = 0; }
            if (b.hp == 0 && b.hasBonus) {
              // Повторное появление подвижного блока ограничим флагом, зато чаще даём второй мяч
              std::uniform_int_distribution<int> d(0, 8);
              BonusType t = static_cast<BonusType>(d(rng()));
              if (t == BonusType::SpawnMovingBlock && movingBlockSpawned) {
                t = BonusType::SpawnSecondBall; // усиливаем вероятность второго мяча
              }
              bonuses.push_back(Bonus{t, {b.rect.left + b.rect.width * 0.5f, b.rect.top + b.rect.height}});
            }
          }
        }
      }
    }

    // Ball-ball collisions (bonus 9 interaction)
    if (balls.size() > 1) {
      if (circleCircleIntersect(balls[0].pos, balls[0].radius, balls[1].pos, balls[1].radius)) {
        // simple elastic response with slight angle diversification
        sf::Vector2f v0 = balls[0].vel, v1 = balls[1].vel;
        std::swap(balls[0].vel, balls[1].vel);
        enforceAngle(balls[0].vel);
        enforceAngle(balls[1].vel);
        // separate balls to avoid long following
        sf::Vector2f dir = balls[0].pos - balls[1].pos;
        float len = std::sqrt(dir.x*dir.x + dir.y*dir.y) + 1e-3f;
        dir /= len;
        balls[0].pos += dir * 2.f;
        balls[1].pos -= dir * 2.f;
      }
    }

    // Bonuses update
    for (auto& bo : bonuses) {
      if (bo.taken) continue;
      bo.pos.y += bo.speed * dt;
      if (bo.pos.y > windowH + 40) bo.taken = true;
      if (paddleRect.contains(bo.pos)) {
        bo.taken = true;
        applyBonus(paddle, balls, blocks, oneTimeFloor, bo.type, window.getSize(), movingBlockSpawned);
      }
    }

    // Win condition: all destructible destroyed
    bool anyDestructible = false;
    for (auto& b : blocks) if (b.hp != 0 && b.hp > 0) { anyDestructible = true; break; }
    if (!anyDestructible) {
      // simple next level: re-randomize bonuses
      for (auto& b : blocks) {
        if (b.hp < 0) continue;
        b.hp = 1;
        b.hasBonus = (std::uniform_int_distribution<int>(0, 1)(rng()) == 1);
      }
      for (auto& b : balls) { b.attached = true; b.vel = {220.f, -260.f}; }
      oneTimeFloor.reset();
    }

    // Render
    window.clear(sf::Color(20, 22, 30));

    // Draw blocks
    for (auto& b : blocks) {
      if (b.hp == 0) continue;
      sf::RectangleShape rs({b.rect.width, b.rect.height});
      rs.setPosition(b.rect.left, b.rect.top);
      if (b.moving) rs.setFillColor(sf::Color(140, 200, 200));
      else if (b.hp < 0) rs.setFillColor(sf::Color(120, 120, 120));
      else if (b.onHitSpeedDelta > 0) rs.setFillColor(sf::Color(220, 120, 60));
      else if (b.hp == 2) rs.setFillColor(sf::Color(80, 170, 240));
      else rs.setFillColor(sf::Color(100, 200, 140));
      window.draw(rs);
    }

    // Draw paddle
    sf::RectangleShape paddleShape({paddle.width, paddle.height});
    paddleShape.setOrigin(paddle.width * 0.5f, paddle.height * 0.5f);
    paddleShape.setPosition(paddle.pos);
    paddleShape.setFillColor(paddle.sticky ? sf::Color(240, 220, 100) : sf::Color(240, 240, 240));
    window.draw(paddleShape);

    // Draw balls
    for (auto const& ball : balls) {
      sf::CircleShape circle(ball.radius);
      circle.setOrigin(ball.radius, ball.radius);
      circle.setPosition(ball.pos);
      circle.setFillColor(sf::Color(240, 240, 240));
      window.draw(circle);
    }

    // Draw one-time floor indicator
    if (oneTimeFloor && *oneTimeFloor) {
      sf::RectangleShape floorLine({static_cast<float>(windowW), 3.f});
      floorLine.setPosition(0.f, windowH - 3.f);
      floorLine.setFillColor(sf::Color(240, 180, 70));
      window.draw(floorLine);
    }

    // Draw bonuses
    for (auto& bo : bonuses) {
      if (bo.taken) continue;
      sf::CircleShape s(6.f);
      s.setOrigin(6.f, 6.f);
      s.setPosition(bo.pos);
      switch (bo.type) {
        case BonusType::ExpandPaddle: s.setFillColor(sf::Color(120, 220, 120)); break;
        case BonusType::CompressPaddle: s.setFillColor(sf::Color(220, 120, 120)); break;
        case BonusType::SlowBall: s.setFillColor(sf::Color(120, 120, 220)); break;
        case BonusType::FastBall: s.setFillColor(sf::Color(220, 180, 60)); break;
        case BonusType::Sticky: s.setFillColor(sf::Color(220, 220, 120)); break;
        case BonusType::OneTimeFloor: s.setFillColor(sf::Color(200, 140, 80)); break;
        case BonusType::RandomBounce: s.setFillColor(sf::Color(180, 120, 220)); break;
        case BonusType::SpawnMovingBlock: s.setFillColor(sf::Color(140, 200, 200)); break;
        case BonusType::SpawnSecondBall: s.setFillColor(sf::Color(220, 120, 200)); break;
      }
      window.draw(s);
    }

    // HUD: lives as circles and score as simple 7-seg like bars
    // Lives
    for (int i = 0; i < std::max(0, lives); ++i) {
      sf::CircleShape life(6.f);
      life.setOrigin(6.f, 6.f);
      life.setPosition(14.f + i * 18.f, 14.f);
      life.setFillColor(sf::Color(230, 90, 90));
      window.draw(life);
    }
    // Score: draw small rectangles representing digits (no font dependency)
    auto drawDigit = [&](int d, sf::Vector2f topLeft) {
      float w = 10.f, h = 16.f, t = 2.f; // segment thickness
      // segments: 0 top, 1 top-left, 2 top-right, 3 middle, 4 bottom-left, 5 bottom-right, 6 bottom
      bool seg[10][7] = {
        {true, true, true, false, true, true, true},   // 0
        {false, false, true, false, false, true, false}, // 1
        {true, false, true, true, true, false, true},  // 2
        {true, false, true, true, false, true, true},  // 3
        {false, true, true, true, false, true, false}, // 4
        {true, true, false, true, false, true, true},  // 5
        {true, true, false, true, true, true, true},   // 6
        {true, false, true, false, false, true, false},// 7
        {true, true, true, true, true, true, true},    // 8
        {true, true, true, true, false, true, true}    // 9
      };
      sf::Color col(240, 240, 240);
      auto drawH = [&](sf::Vector2f p) { sf::RectangleShape r({w, t}); r.setPosition(p); r.setFillColor(col); window.draw(r); };
      auto drawV = [&](sf::Vector2f p) { sf::RectangleShape r({t, h}); r.setPosition(p); r.setFillColor(col); window.draw(r); };
      if (seg[d][0]) drawH({topLeft.x, topLeft.y});
      if (seg[d][1]) drawV({topLeft.x, topLeft.y});
      if (seg[d][2]) drawV({topLeft.x + w - t, topLeft.y});
      if (seg[d][3]) drawH({topLeft.x, topLeft.y + h});
      if (seg[d][4]) drawV({topLeft.x, topLeft.y + h});
      if (seg[d][5]) drawV({topLeft.x + w - t, topLeft.y + h});
      if (seg[d][6]) drawH({topLeft.x, topLeft.y + 2.f * h});
    };
    auto drawNumber = [&](int value, sf::Vector2f topLeft) {
      if (value < 0) value = 0;
      std::string s = std::to_string(value);
      for (std::size_t i = 0; i < s.size(); ++i) {
        int d = s[i] - '0';
        drawDigit(d, {topLeft.x + static_cast<float>(i) * 14.f, topLeft.y});
      }
    };
    drawNumber(score, {windowW - 100.f, 8.f});

    window.display();
  }
  return 0;
}


