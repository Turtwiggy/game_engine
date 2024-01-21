#include "system.hpp"

#include "entt/helpers.hpp"
#include "imgui/helpers.hpp"
#include "modules/items/helpers.hpp"
#include "modules/items_pickup/components.hpp"
#include "modules/lifecycle/components.hpp"

#include "imgui.h"

#include <string>

namespace game2d {
using namespace std::literals;

void
update_ui_inventory(entt::registry& r)
{
  auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);

  ImGui::Begin("Inventory");
  {
    const auto& view = r.view<ItemComponent, HasParentComponent>(entt::exclude<WaitForInitComponent>);

    for (const auto& [entity, item, parent] : view.each()) {

      const auto info = item_id_to_sprite(r, item.item_id);
      ImGui::Text("%s", info.display.c_str());

      ImGui::SameLine();

      if (ImGui::Button(append_eid_to_label("delete", entity).c_str()))
        dead.dead.emplace(entity);
    }
  }
  ImGui::End();
}

} // namespace game2d