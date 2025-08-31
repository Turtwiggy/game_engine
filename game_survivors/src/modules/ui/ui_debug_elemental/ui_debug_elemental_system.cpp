#include "pch.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/combat/combat_elemental_damage/elemental_damage_components.hpp"
#include "modules/combat/combat_flamethrower/flamethrower_components.hpp"
#include "modules/core/camera/helpers.hpp"
#include "modules/core/renderer/components.hpp"
#include "ui_debug_elemental_system.hpp"

namespace game2d {

void
update_ui_debug_elemental_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  const auto& ri = SINGLE_RendererInfo::instance;

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_NoMove;
  flags |= ImGuiWindowFlags_NoBackground;
  flags |= ImGuiWindowFlags_NoFocusOnAppearing;
  flags |= ImGuiWindowFlags_NoDocking;
  flags |= ImGuiWindowFlags_NoInputs;
  flags |= ImGuiWindowFlags_NoSavedSettings;

  const auto screen_size = ImVec2{ (float)ri.viewport_size_render_at.x, (float)ri.viewport_size_render_at.y };
  ImGui::SetNextWindowPos({ 0, 0 }, ImGuiCond_Always, { 0.0f, 0.0f });
  ImGui::SetNextWindowSize(screen_size, ImGuiCond_Always);
  ImGui::Begin("overlay", NULL, flags);

  for (const auto& [e, parent_c, tick_c] : r.view<const HasParentComponent, const TickDamageComponent>().each()) {

    if (parent_c.parent == entt::null || !r.valid(parent_c.parent))
      continue;

    const auto eid = static_cast<uint32_t>(e);
    ImGui::PushID(eid);

    const auto pos = get_position(r, parent_c.parent);
    const auto wsp = glm::vec2(pos.x, pos.y);
    const auto wsp_adj = glm::vec2{ wsp.x, wsp.y };
    const auto ss_pos = worldspace_to_screenspace(r, wsp_adj);
    ImGui::SetCursorScreenPos({ ss_pos.x, ss_pos.y });

    const int fire_stacks = (int)tick_c.fire.size();
    const int ice_stacks = (int)tick_c.ice.size();
    const int shock_stacks = (int)tick_c.shock.size();
    const int poison_stacks = (int)tick_c.poison.size();

    std::string text = "";
    if (fire_stacks > 0)
      text += " f:" + std::to_string(fire_stacks);
    if (ice_stacks > 0)
      text += " i:" + std::to_string(ice_stacks);
    if (shock_stacks > 0)
      text += " s:" + std::to_string(shock_stacks);
    if (poison_stacks > 0)
      text += " p:" + std::to_string(poison_stacks);

    ImGui::Text("%s", text.c_str());

    ImGui::PopID();
  }

  for (const auto& [e, weapon_c, parent_c] :
       r.view<const FlamethrowerFlameFixtureComponent, const HasParentComponent>().each()) {

    if (parent_c.parent == entt::null || !r.valid(parent_c.parent))
      continue;

    const auto eid = static_cast<uint32_t>(e);
    ImGui::PushID(eid);

    const auto pos = get_position(r, parent_c.parent);
    const auto wsp = glm::vec2(pos.x, pos.y);
    const auto wsp_adj = glm::vec2{ wsp.x, wsp.y };
    const auto ss_pos = worldspace_to_screenspace(r, wsp_adj);
    ImGui::SetCursorScreenPos({ ss_pos.x, ss_pos.y });

    const int burning_targets = (int)weapon_c.burning_fixture_es.size();
    ImGui::Text("bt:%i", burning_targets);

    ImGui::PopID();
  }

  ImGui::End();
}

} // namespace game2d