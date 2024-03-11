#include "system.hpp"

#include "actors.hpp"
#include "components.hpp"
#include "entt/helpers.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/actor_weapon_shotgun/components.hpp"
#include "modules/combat_attack_cooldown/components.hpp"
#include "modules/lifecycle/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/scene/helpers.hpp"
#include "physics/components.hpp"

#include "imgui.h"

namespace game2d {

void
update_ui_level_up_system(entt::registry& r)
{
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  auto& levelup = get_first_component<SINGLE_UILevelUpComponent>(r);

  // HACK: test leveling up window.
  const auto& first_player_e = get_first<PlayerComponent>(r);
  auto& first_player = r.get<PlayerComponent>(first_player_e);
  if (first_player.picked_up_xp >= 10)
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

  ImGui::Begin("Level Up", &levelup.show_menu, flags);

  if (ImGui::BeginTable("level-up-table", 3)) {
    ImGui::TableNextRow();

    ImGui::TableNextColumn();
    const auto& weapon = first_player.weapon;
    auto& weapon_info = r.get<WeaponBulletTypeToSpawnComponent>(weapon);
    auto& weapon_cooldown = r.get<AttackCooldownComponent>(weapon);

    ImGui::Text("Current Damage: %f", weapon_info.bullet_damage);
    if (ImGui::Button("GAIN DAMAGE\n+1 Damage", ImVec2(-FLT_MIN, -FLT_MIN))) {

      weapon_info.bullet_damage += 1.0f;

      // HACK
      first_player.picked_up_xp -= 10;
      levelup.show_menu = false;
    }

    ImGui::TableNextColumn();
    ImGui::Text("Current Firerate: %f", weapon_cooldown.time_between_attack);
    if (ImGui::Button("GAIN FIRERATE\n+0.01 Firerate", ImVec2(-FLT_MIN, -FLT_MIN))) {
      weapon_cooldown.time_between_attack -= 0.1;
      weapon_cooldown.time_between_attack = glm::max(weapon_cooldown.time_between_attack, 0.1f);

      // HACK
      first_player.picked_up_xp -= 10;
      levelup.show_menu = false;
    }

    ImGui::TableNextColumn();
    if (ImGui::Button("GAIN UNIT\n+1 Unit", ImVec2(-FLT_MIN, -FLT_MIN))) {
      const auto first_player_group = r.get<HasParentComponent>(first_player_e).parent;
      const auto new_player = create_player(r, first_player_group);

      // HACK
      first_player.picked_up_xp -= 10;
      levelup.show_menu = false;
    }

    ImGui::EndTable();
  }
  ImGui::End();
}

} // namespace game2d