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
#include "engine/maths/line.hpp"
#include "modules/core/raws/raws_components.hpp"
#include "modules/core/sprites/sprite_helpers.hpp"
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
update_actor_rocks_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  static float cutoff = 0.69f; // [0, 1]

#if defined(_DEBUG)
  auto& input_c = get_first_component<SINGLE_InputComponent>(r);
  if (get_key_down(input_c, SDL_SCANCODE_KP_7))
    create_empty<RequestGenerateRocks>(r);

  auto& menu_c = get_first_component<SINGLE_DebugMenuBar>(r);
  auto rock_ui = gesert_menubar_state(menu_c, "actor_rocks");
  if (rock_ui.enabled)
    imgui_draw_float("rock_cutoff", cutoff);

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

    // Destroy all the old rocks.
    auto& dead_c = get_first_component<SINGLE_EntityBinComponent>(r);
    const auto view = r.view<RockComponent>();
    for (const auto& [e, rock_c] : view.each())
      dead_c.dead.push_back(e);

    // clear the id <=> eid map
    SINGLE_Islands::instance.id_to_island_eid.clear();

    generate_rocks(r, cutoff);
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