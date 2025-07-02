#pragma once

#include <entt/fwd.hpp>

namespace game2d {

struct RequestQuip
{
  entt::entity thing_to_quip;
};

struct SINGLE_QuipSystem
{
  float time_to_quip_sec_cur = 0.0;
  const float time_to_quip_sec_min = 30;     // 30 seconds
  const float time_to_quip_sec_max = 60 * 1; // 1 minute
};

} // namespace game2d