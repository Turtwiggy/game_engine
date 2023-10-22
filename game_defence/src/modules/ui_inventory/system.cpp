#include "system.hpp"

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
  ImGui::Begin("Inventory");
  {
    const auto& view = r.view<ItemComponent, HasParentComponent>(entt::exclude<WaitForInitComponent>);

    for (const auto& [entity, item, parent] : view.each()) {

      const auto info = item_id_to_sprite(r, item.item_id);
      ImGui::Text("%s", info.display.c_str());

      ImGui::SameLine();

      auto eid = static_cast<uint32_t>(entity);
      std::string label = "delete##"s + std::to_string(eid);
      if (ImGui::Button(label.c_str()))
        r.destroy(entity);
    }
  }
  ImGui::End();
}

} // namespace game2d