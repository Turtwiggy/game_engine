#include "system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/physics/physics_components.hpp"

#include "imgui.h"
#include "modules/core_renderer/components.hpp"

namespace game2d {

void
update_ui_collisions_system(entt::registry& r)
{
  const auto& physics = get_first_component<SINGLE_Physics>(r);
  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);
  const auto pos = glm::vec2{ 0, ri.viewport_size_render_at.y - (ImGui::GetFontSize() * 2.0f) }; // bl

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_NoBackground;
  flags |= ImGuiWindowFlags_NoDocking;
  flags |= ImGuiWindowFlags_NoSavedSettings;
  flags |= ImGuiWindowFlags_NoFocusOnAppearing;
  flags |= ImGuiWindowFlags_AlwaysAutoResize;
  flags |= ImGuiWindowFlags_NoMove;
  flags |= ImGuiWindowFlags_NoInputs;

  const auto offset = ImVec2{ 100, 0 };
  ImGui::SetNextWindowPos(ImVec2{ pos.x + offset.x, pos.y }, ImGuiCond_Always, { 0, 0 });

  ImGui::Begin("Collisions", NULL, flags);

  ImGui::SameLine();
  ImGui::Text("BodyCount(): %i", physics.world->GetBodyCount());

  ImGui::SameLine();
  ImGui::Text("ContactCount(): %i", physics.world->GetContactCount());

  ImGui::End();
}

} // namespace game2d