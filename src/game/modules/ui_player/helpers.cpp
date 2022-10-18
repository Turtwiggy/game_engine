#include "helpers.hpp"

#include "game/modules/combat/components.hpp"

#include <imgui.h>

namespace game2d {

void
draw_healthbar(Game& game, const entt::entity& entity)
{
  auto& r = game.state;

  ImDrawList* draw_list = ImGui::GetWindowDrawList();

  const auto& health = r.get<HealthComponent>(entity);
  float percent = health.hp / (float)health.max_hp;

  ImVec2 canvas_p0 = ImGui::GetCursorScreenPos(); // ImDrawList API uses screen coordinates!
  ImVec2 canvas_sz = ImGui::GetContentRegionAvail();
  if (canvas_sz.x < 50.0f)
    canvas_sz.x = 50.0f;
  if (canvas_sz.y < 50.0f)
    canvas_sz.y = 50.0f;
  canvas_sz.x *= percent;
  canvas_sz.y = 12.0f;
  ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);
  draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(200, 25, 25, 255));

  // ImGui::SameLine(0.0f);

  ImGui::Text("HP: %i / %i", health.hp, health.max_hp);
}

} // namespace game2d