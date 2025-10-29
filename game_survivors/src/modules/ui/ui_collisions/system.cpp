#include "pch.hpp"

#include "system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/imgui/ui_imgui_defaults.hpp"
#include "engine/physics/physics_components.hpp"
#include "modules/core/renderer/components.hpp"

namespace game2d {

void
update_ui_collisions_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  const auto& physics = get_first_component<SINGLE_Physics>(r);
  const auto& ri = SINGLE_RendererInfo::instance;

  if (!b2World_IsValid(physics.worldId))
    return;

  const auto pos = glm::vec2{ 0, ri.viewport_size_render_at.y - (ImGui::GetFontSize() * 2.0f) }; // bl
  const auto offset = ImVec2{ 100, 0 };

  ImGui::SetNextWindowPos(ImVec2{ pos.x + offset.x, pos.y }, ImGuiCond_Always, { 0, 0 });

  imgui_begin("Collisions", ImGuiWindowFlags_NoInputs);

  ImGui::SameLine();
  ImGui::Text("BodyCount(): %i ", b2World_GetAwakeBodyCount(physics.worldId));

  ImGui::SameLine();
  const b2SensorEvents s_events = b2World_GetSensorEvents(physics.worldId);
  const b2ContactEvents c_events = b2World_GetContactEvents(physics.worldId);
  ImGui::Text("BeginContactCount(): %i BeginSensorEvents(): %i", c_events.beginCount, s_events.beginCount);

  ImGui::End();
}

} // namespace game2d