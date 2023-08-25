#include "system.hpp"

#include "actors.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "events/helpers/mouse.hpp"
#include "imgui/helpers.hpp"
#include "lifecycle/components.hpp"
#include "maths/grid.hpp"
#include "modules/actor_bow/components.hpp"
#include "modules/actor_hearth/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/actor_turret/components.hpp"
#include "modules/camera/helpers.hpp"
#include "modules/combat/components.hpp"
#include "modules/ui_hierarchy/helpers.hpp"
#include "renderer/components.hpp"
#include "sprites/components.hpp"

#include "glm/glm.hpp"
#include "imgui.h"
#include "magic_enum.hpp"

#include <string>

namespace game2d {

void
update_ui_economy_system(entt::registry& r)
{
  ImGui::Begin("Bow UI");
  const auto& view = r.view<BowComponent>();
  for (const auto& [entity, bow] : view.each()) {
    ImGui::Text("Bow...");
    imgui_draw_float("Bow Lerp Speed: ", bow.lerp_speed);
  }
  ImGui::End();

  auto& econ = get_first_component<SINGLETON_Economy>(r);
  auto& ui_econ = get_first_component<SINGLETON_UiEconomy>(r);
  const glm::ivec2 mouse_position = mouse_position_in_worldspace(r);

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoFocusOnAppearing;
  flags |= ImGuiDockNodeFlags_AutoHideTabBar;

  ImGui::Begin("Economy", NULL, flags);
  ImGui::Text("Money: %i", econ.kills);

  // hack showing player hp
  const auto& first_player = get_first<PlayerComponent>(r);
  if (first_player != entt::null) {
    const auto& player = r.get<PlayerComponent>(first_player);
    const auto& health = r.try_get<HealthComponent>(first_player);
    if (health)
      ImGui::Text("Player HP: %i", health->hp);
    ImGui::Text("Picked up XP: %i", player.picked_up_xp);
    ImGui::Text("Bullets Left: %i", player.bullets_in_clip_left);
  }

  // hack showing hearth hp
  const auto& first_hearth = get_first<HearthComponent>(r);
  if (first_hearth != entt::null) {
    const auto& first_hearth_hp = r.get<HealthComponent>(first_hearth);
    ImGui::Text("Hearth HP: %i", first_hearth_hp.hp);
  }

  // Show turret as selectable
  const std::vector<std::string> items{
    //
    std::string(magic_enum::enum_name(EntityType::actor_turret)),
    std::string(magic_enum::enum_name(EntityType::actor_enemy)),
    //
  };

  // select a type to place
  EntityType type = EntityType::empty;
  {
    static int econ_item_current_idx = 0;
    {
      WomboComboIn in(items);
      in.label = "combo_for_items";
      in.current_index = econ_item_current_idx;
      const auto out = draw_wombo_combo(in);
      econ_item_current_idx = out.selected;
    }
    type = magic_enum::enum_cast<EntityType>(items[econ_item_current_idx]).value();
  }

  // buying turrets
  if (type == EntityType::actor_turret) {
    static int turrets_bought = 0;
    static int turret_cost = 10;
    bool can_buy_turret = econ.kills > turret_cost;
    ImGui::Text("Turret Cost: %i", turret_cost);
    ImGui::Text("Turrets Owned: %i", turrets_bought);
    if (ImGui::Button("Buy Turret##buyturret") && can_buy_turret) {
      econ.kills -= turret_cost;
      turrets_bought++;
    }
  }

  // only show classes if you're placing enemies
  if (type == EntityType::actor_enemy) {
    static int weapon_item_current_idx = 0;
    std::vector<std::string> weapons;
    for (int i = 0; i < static_cast<int>(Weapon::count); i++) {
      Weapon value = magic_enum::enum_value<Weapon>(i);
      std::string value_str = std::string(magic_enum::enum_name(value));
      weapons.push_back(value_str);
    }

    WomboComboIn in(weapons);
    in.label = "combo for weapons";
    in.current_index = weapon_item_current_idx;
    const auto out = draw_wombo_combo(in);
    weapon_item_current_idx = out.selected;

    ImGui::Checkbox("Add Weapon", &ui_econ.add_weapon);
    ui_econ.weapon_to_add = magic_enum::enum_value<Weapon>(weapon_item_current_idx);
  }

  ImGui::End();

  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  if (!ri.viewport_hovered)
    return;

  bool place = get_mouse_rmb_press();
  if (place) {
    CreateEntityRequest request;
    request.type = type;
    request.position = { mouse_position.x, mouse_position.y, 0 };
    r.emplace<CreateEntityRequest>(r.create(), request);
  }


};

} // namespace game2d