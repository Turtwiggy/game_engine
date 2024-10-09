#pragma once

#include <entt/entt.hpp>

namespace game2d {

entt::entity
create_shotgun(entt::registry& r, entt::entity parent);

entt::entity
create_jetpack_player(entt::registry& r);

template<class T>
bool
cleanup_requests(entt::registry& r)
{
  const auto& reqs = r.view<T>();
  const bool had_requests = reqs.size() > 0;
  r.destroy(reqs.begin(), reqs.end()); // done requests
  return had_requests;
};

} // namespace game2d