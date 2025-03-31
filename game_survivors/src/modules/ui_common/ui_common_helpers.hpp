#pragma once

#include "modules/ui_common/ui_common_components.hpp"

#include <entt/fwd.hpp>

#include <functional>

namespace game2d {

template<class T>
void
process_requests(entt::registry& r, const std::function<void(const T& req)>& callback)
{
  const auto& view = r.view<T>();

  const bool has_requests = view.size() > 0;
  if (has_requests)
    callback(r.get<T>(view.front()));

  r.destroy(view.begin(), view.end());
};

bool
selectable_button(entt::registry& r, SelectableButtonDef& def);

} // namespace game2d