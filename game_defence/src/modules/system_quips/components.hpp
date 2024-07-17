#pragma once

#include <entt/entt.hpp>

#include <string>
#include <vector>

namespace game2d {

struct SINGLE_QuipsComponent
{
  std::vector<std::string> quips;
  std::vector<std::string> quips_hit;

  // when pulling from quips,
  // choose from unused,
  // then remove it from the vector.
  // this will make sure you dont pull the same quips over and over
  // when quips_unused is 0, reload from quips

  std::vector<std::string> quips_unused;
  std::vector<std::string> quips_hit_unused;
};

enum class QuipType
{
  ENTER_ROOM,
  TOOK_DAMAGE,
};

struct RequestQuip
{
  QuipType type;
  entt::entity quipp_e = entt::null;
};

} // namespace game2d