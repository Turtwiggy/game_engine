#include "system.hpp"

#include "actors.hpp"
#include "components.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/actor_weapon_shotgun/components.hpp"
#include "modules/combat_attack_cooldown/components.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/items_pickup/components.hpp"
#include "modules/renderer/components.hpp"

#include "imgui.h"

namespace game2d {

void
update_ui_level_up_system(entt::registry& r)
{
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  auto& levelup = get_first_component<SINGLE_UILevelUpComponent>(r);
  auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);

  entt::entity first_player = entt::null;
  int first_player_xp = 0;
  std::map<EntityType, std::vector<entt::entity>> compacted_items;

  // HACK: test leveling up window.
  //
  const auto& player_view = r.view<const PlayerComponent>(entt::exclude<WaitForInitComponent>);
  const auto& items_view =
    r.view<const ItemComponent, const HasParentComponent, const EntityTypeComponent>(entt::exclude<WaitForInitComponent>);
  for (const auto& [player_e, player_c] : player_view.each()) {
    //
    // Show like potion x1, potion x2 not potions individually
    for (const auto& [item_e, item_c, item_parent, item_type] : items_view.each()) {
      if (item_parent.parent != player_e)
        continue; // not my item
      compacted_items[item_type.type].push_back(item_e);
    }

    first_player = player_e;
    if (compacted_items.find(EntityType::actor_pickup_xp) != compacted_items.end())
      first_player_xp = compacted_items[EntityType::actor_pickup_xp].size();
    break;
  }

  // Some checks
  if (first_player == entt::null)
    return;
  if (first_player_xp >= 10)
    levelup.show_menu = true;
  if (!levelup.show_menu)
    return;

  const auto& viewport_pos = ImVec2(ri.viewport_pos.x, ri.viewport_pos.y);
  const auto& viewport_size_half = ImVec2(ri.viewport_size_current.x * 0.5f, ri.viewport_size_current.y * 0.5f);
  const auto pos = ImVec2(viewport_pos.x + viewport_size_half.x, viewport_pos.y + viewport_size_half.y);
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSizeConstraints(ImVec2(640, 640 * (9 / 16.0f)), ImVec2(640, 640 * (9 / 16.0f)));

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoMove;
  flags |= ImGuiWindowFlags_NoCollapse;
  flags |= ImGuiWindowFlags_NoTitleBar;
  // flags |= ImGuiWindowFlags_NoBackground;

  const auto& use_xp_items = [&r, &compacted_items, &dead](int amount) {
    if (compacted_items.find(EntityType::actor_pickup_xp) != compacted_items.end()) {
      auto& xp_items = compacted_items[EntityType::actor_pickup_xp];
      for (int i = 0; const auto& xp : xp_items) {
        if (i == (amount))
          break;
        i++;
        r.remove<ItemComponent>(xp);
        dead.dead.emplace(xp);
      }
    }
  };

  ImGui::Begin("Level Up", &levelup.show_menu, flags);

  if (ImGui::BeginTable("level-up-table", 3)) {
    ImGui::TableNextRow();

    ImGui::TableNextColumn();
    const auto& player_weapon = r.get<HasWeaponComponent>(first_player);
    const auto& weapon = player_weapon.instance;
    auto& weapon_info = r.get<WeaponBulletTypeToSpawnComponent>(weapon);
    auto& weapon_cooldown = r.get<AttackCooldownComponent>(weapon);
    ImGui::Text("Current Damage: %f", weapon_info.bullet_damage);
    if (ImGui::Button("GAIN DAMAGE\n+1 Damage", ImVec2(-FLT_MIN, -FLT_MIN))) {
      weapon_info.bullet_damage += 1.0f;
      use_xp_items(10);
      levelup.show_menu = false;
    }

    ImGui::TableNextColumn();
    ImGui::Text("Current Firerate: %f", weapon_cooldown.time_between_attack);
    if (ImGui::Button("GAIN FIRERATE\n+0.01 Firerate", ImVec2(-FLT_MIN, -FLT_MIN))) {
      weapon_cooldown.time_between_attack -= 0.1;
      weapon_cooldown.time_between_attack = glm::max(weapon_cooldown.time_between_attack, 0.1f);
      use_xp_items(10);
      levelup.show_menu = false;
    }

    ImGui::TableNextColumn();
    if (ImGui::Button("GAIN UNIT\n+1 Unit", ImVec2(-FLT_MIN, -FLT_MIN))) {
      std::cout << "TODO: implement gaining a unit" << std::endl;

      // const auto first_player_group = r.get<HasParentComponent>(first_player).parent;
      // const auto new_player = create_player(r, first_player_group);
      use_xp_items(10);
      levelup.show_menu = false;
    }

    ImGui::EndTable();
  }
  ImGui::End();
}

} // namespace game2d