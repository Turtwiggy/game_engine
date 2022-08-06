#pragma once

#include <entt/entt.hpp>

#include <string>

namespace game2d {

struct Item
{
  std::string name;
  int quantity = 0;

  // C++ Core Guidelines: C.67
  // A polymorphic class should suppress public copy/move.
  // https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#c67-a-polymorphic-class-should-suppress-public-copymove
  Item() = default;
  Item(const Item&) = delete;
  Item& operator=(const Item&) = delete;

  virtual bool use(entt::registry& r) { return false; };
};

} // namespace game2d