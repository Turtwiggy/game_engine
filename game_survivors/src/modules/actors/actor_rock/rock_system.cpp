#include "pch.hpp"

#include "rock_system.hpp"

#include "rock_components.hpp"
#include "rock_helpers.hpp"

#include "engine/colour/colour.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "modules/core/ui/ui_common_helpers.hpp"
#include "modules/ui/ui_debug_menubar/ui_debug_menubar_helpers.hpp"

namespace game2d {

void
update_actor_rocks_system(entt::registry& r)
{
  static float cutoff = 0.70f;

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
    // Destroy all the old rocks.
    auto& dead_c = get_first_component<SINGLE_EntityBinComponent>(r);
    const auto view = r.view<RockComponent>();
    for (const auto& [e, rock_c] : view.each())
      dead_c.dead.push_back(e);

    generate_rocks(r, cutoff);
  });
}

} // namespace game2d