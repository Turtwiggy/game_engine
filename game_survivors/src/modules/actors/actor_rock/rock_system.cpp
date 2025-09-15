#include "pch.hpp"

#include "rock_components.hpp"
#include "rock_helpers.hpp"
#include "rock_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/colour/colour.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/grid.hpp"
#include "engine/maths/line.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/core/raws/raws_components.hpp"
#include "modules/core/ui/ui_common_helpers.hpp"
#include "modules/ui/ui_debug_menubar/ui_debug_menubar_helpers.hpp"

namespace game2d {

void
draw_rocks(entt::registry& r)
{
  int i = 0;

  for (const auto& [e, debug_c] : r.view<const DebugContoursComponent>().each()) {

    for (const auto& edge : debug_c.sorted_edges) {

      // give each island a different colour
      const auto tmp = (ImVec4)ImColor::HSV(i / 7.0f, 0.6f, 0.6f);
      const engine::SRGBColour col{ tmp.x, tmp.y, tmp.z, tmp.w };

      const auto line_info = generate_line(edge.a, edge.b, 4.0f);
      const auto spawned_e = spawn(r, "empty");
      give_life(r, spawned_e, { 0, 0 }, { 0, 0 });
      set_position_and_size_with_line(r, spawned_e, line_info);
      set_colour(r, spawned_e, col);
    }

    i++;
  }
};

void
update_actor_rocks_system(entt::registry& r, glm::vec2 mouse_pos)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  auto& menu_c = get_first_component<SINGLE_DebugMenuBar>(r);
  auto rock_ui = gesert_menubar_state(menu_c, "DebugIslands");
  if (rock_ui.enabled) {
    ImGui::Begin("DebugIslands");
    imgui_draw_float("frequency", SINGLE_Islands::instance.frequency);
    imgui_draw_float("cutoff", SINGLE_Islands::instance.cutoff);
    // ImGui::Text("valid: %d", SINGLE_Islands::instance.valid);

    const auto wh = SINGLE_Islands::instance.wh;
    const auto tilesize = SINGLE_Islands::instance.tilesize;
    const auto offset = (int)(-wh * 0.5f);
    const auto offset_worldspace = (int)offset * tilesize;
    const glm::vec2 offset_mouse_pos = mouse_pos - glm::vec2{ offset_worldspace, offset_worldspace };

    const auto& generated = SINGLE_Islands::instance.generated;
    auto mouse_idx = engine::grid::worldspace_to_index({ offset_mouse_pos.x, offset_mouse_pos.y },
                                                       SINGLE_Islands::instance.tilesize,
                                                       SINGLE_Islands::instance.wh,
                                                       SINGLE_Islands::instance.wh);
    mouse_idx = glm::clamp(mouse_idx, 0, glm::max((int)generated.size() - 1, 0));

    ImGui::Text("mouse: %f, %f", mouse_pos.x, mouse_pos.y);
    ImGui::Text("offset mousepos: %f, %f", offset_mouse_pos.x, offset_mouse_pos.y);
    ImGui::Text("mouse idx: %i", mouse_idx);

    if (generated.size() > 0)
      ImGui::Text(
        "noise at (%i %i), idx: %f", generated[mouse_idx].xy.x, generated[mouse_idx].xy.y, generated[mouse_idx].noise);

    ImGui::End();
  }

#if defined(_DEBUG)
  auto& input_c = get_first_component<SINGLE_InputComponent>(r);
  if (get_key_down(input_c, SDL_SCANCODE_KP_7)) {

#if defined(_DEBUG)
    // Destroy all the old rocks.
    for (const auto& [e, rock_c] : r.view<const RockComponent>().each())
      r.destroy(e);
    const auto view = r.view<TransformComponent>(entt::exclude<Persistent>);
    for (const auto& [e, t_c] : view.each())
      r.destroy(e);
#endif

    create_empty<RequestGenerateRocks>(r);
  }

  /*
static float im_red[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
static float im_green[4]{ 0.0f, 1.0f, 0.0f, 1.0f };
static float percent = 0.0f;
ImGui::ColorEdit4("red", im_red);
ImGui::ColorEdit4("green", im_green);
ImGui::SliderFloat("mix", &percent, 0.0f, 1.0f);

static engine::SRGBColour my_r;
static engine::SRGBColour my_g;
my_r = { im_red[0], im_red[1], im_red[2], im_red[3] };
my_g = { im_green[0], im_green[1], im_green[2], im_green[3] };
const auto lerped = lerp_colour(my_r, my_g, percent);

static float im_lerp[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
im_lerp[0] = lerped.r / 255.0f;
im_lerp[1] = lerped.g / 255.0f;
im_lerp[2] = lerped.b / 255.0f;
im_lerp[3] = lerped.a / 255.0f;
ImGui::ColorEdit4("mixed_col", im_lerp);
*/
#endif

  process_requests<RequestGenerateRocks>(r, [&](const auto& req) {
    SDL_Log("Request to generate rocks...");

    // clear the id <=> eid map
    SINGLE_Islands::instance.id_to_island_eid.clear();

    generate_rocks(r);
    draw_rocks(r);
  });

  // int j = 0;
  // const auto& input_c = get_first_component<SINGLE_InputComponent>(r);
  // static int debug_island = 0;
  // static int debug_edge = 0;
  // imgui_draw_int("debug_island", debug_island);
  // imgui_draw_int("debug_edge", debug_edge);
  // if (get_key_down(input_c, SDL_SCANCODE_KP_PLUS))
  //   debug_edge++;
  // if (get_key_down(input_c, SDL_SCANCODE_KP_MINUS))
  //   debug_edge--;
}

} // namespace game2d