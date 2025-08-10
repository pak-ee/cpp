#pragma once

#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>

inline float clamp(float v, float lo, float hi) {
  return std::max(lo, std::min(v, hi));
}

inline sf::Vector2f reflectHorizontal(const sf::Vector2f& v) {
  return sf::Vector2f{-v.x, v.y};
}

inline sf::Vector2f reflectVertical(const sf::Vector2f& v) {
  return sf::Vector2f{v.x, -v.y};
}

inline bool circleRectIntersect(const sf::FloatRect& r, const sf::Vector2f& c, float radius, sf::Vector2f* outNormal) {
  float cx = clamp(c.x, r.left, r.left + r.width);
  float cy = clamp(c.y, r.top, r.top + r.height);
  float dx = c.x - cx;
  float dy = c.y - cy;
  float dist2 = dx * dx + dy * dy;
  if (dist2 > radius * radius) return false;
  float leftPen = std::abs((c.x + radius) - r.left);
  float rightPen = std::abs((r.left + r.width) - (c.x - radius));
  float topPen = std::abs((c.y + radius) - r.top);
  float bottomPen = std::abs((r.top + r.height) - (c.y - radius));
  float minPen = std::min({leftPen, rightPen, topPen, bottomPen});
  if (minPen == leftPen) *outNormal = sf::Vector2f{-1.f, 0.f};
  else if (minPen == rightPen) *outNormal = sf::Vector2f{1.f, 0.f};
  else if (minPen == topPen) *outNormal = sf::Vector2f{0.f, -1.f};
  else *outNormal = sf::Vector2f{0.f, 1.f};
  return true;
}

inline bool circleCircleIntersect(const sf::Vector2f& aPos, float aR, const sf::Vector2f& bPos, float bR) {
  sf::Vector2f d = aPos - bPos;
  float r = aR + bR;
  return (d.x * d.x + d.y * d.y) <= r * r;
}


