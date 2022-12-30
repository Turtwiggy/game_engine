#include "helpers.hpp"

#include "modules/combat/components.hpp"

#include <entt/entt.hpp>
#include <imgui.h>

namespace game2d {

std::array<ImVec2, 2>
convert_sprite_to_uv(GameEditor& editor, Game& game, const int x, const int y)
{
  const int size_x = 768;
  const int size_y = 352;
  const int cols_x = 48;
  const int cols_y = 22;
  const int pixels_x = size_x / cols_x;
  const int pixels_y = size_y / cols_y;

  // these are for the full texture
  // ImVec2 tl = ImVec2(0.0f, 0.0f);
  // ImVec2 br = ImVec2(1.0f, 1.0f);

  // this is for part of the texture
  const float offset_x = x;
  const float offset_y = y;
  const ImVec2 tl = { (offset_x * pixels_x + 0.0f) / size_x, (offset_y * pixels_y + 0.0f) / size_y };
  const ImVec2 br = { (offset_x * pixels_x + pixels_x) / size_x, (offset_y * pixels_y + pixels_x) / size_y };
  return { tl, br };
}

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