#pragma once

#include "modules/ui_common/ui_common_components.hpp"

#include <entt/entt.hpp>

#include <functional>

namespace game2d {

template<class T>
void
process_requests(entt::registry& r, const std::function<void()>& callback)
{
  bool has_requests = false;
  const auto& view = r.view<T>();

  if (view.size() > 0)
    has_requests = true;
  r.destroy(view.begin(), view.end());

  if (has_requests)
    callback();
};

bool
selectable_button(SelectableButtonDef& def);

} // namespace game2d