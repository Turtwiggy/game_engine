#pragma once

#include "modules/actors/actor_player/components.hpp"
#include "modules/core/ui/ui_common_components.hpp"

#include <entt/fwd.hpp>

#include <functional>

namespace game2d {

template<class T>
void
process_requests(entt::registry& r, const std::function<void(const T& req)>& callback)
{
  const auto& view = r.view<T>();

  const bool has_requests = !view.empty();
  if (has_requests)
    callback(r.get<T>(view.front()));

  r.destroy(view.begin(), view.end());
};

bool
selectable_button(entt::registry& r, SelectableButtonDef& def);

void
process_input_for_ui_all_handles(entt::registry& r, UIState& state);

void
process_input_for_ui(entt::registry& r, UIState& state, const InputComponent& inp_c);

void
clear_navlinks(std::vector<std::shared_ptr<Cell>>& cells);

void
create_as_vertical_layout(std::vector<std::shared_ptr<Cell>>& cells);

void
create_as_horizontal_layout(std::vector<std::shared_ptr<Cell>>& cells);

} // namespace game2d