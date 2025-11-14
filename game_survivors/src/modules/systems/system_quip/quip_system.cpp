#include "pch.hpp"

#include "quip_components.hpp"
#include "quip_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/maths/maths.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/combat/combat_core/components.hpp"
#include "modules/core/animations/wiggle/components.hpp"
#include "modules/core/fonts/fonts_helpers.hpp"
#include "modules/core/ui/ui_common_helpers.hpp"
#include "modules/systems/system_island_ai/island_ai_components.hpp"
#include "modules/ui/ui_scene_survive_land_on_islands_popups/ui_land_on_island_popup_helpers.hpp"
#include "modules/ui/ui_worldspace_text/components.hpp"
#include "resources/data.hpp"

namespace game2d {

void
update_quip_system(entt::registry& r, const float dt)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  // #if defined(_DEBUG)
  //   static engine::RandomState rnd_quip(0); // same roll every time
  // #else
  //   static engine::RandomState rnd_quip(engine::get_system_time_for_seed());
  // #endif

#if defined(_DEBUG)
  // const auto& input_c = get_first_component<SINGLE_InputComponent>(r);
  // if (get_key_down(input_c, SDL_SCANCODE_8)) {
  //   const auto view = r.view<const IslanderAiComponent, TeamComponent>();
  //   for (const auto& [e, islander_ai_c, team_c] : view.each()) {
  //     if (team_c.team == AvailableTeams::enemy)
  //       continue;
  //     auto thing_to_quip = e;
  //     create_empty<RequestQuip>(r, RequestQuip{ .thing_to_quip = thing_to_quip });
  //     break;
  //   }
  // }
#endif

  process_requests<RequestQuip>(r, [&](const RequestQuip& req) {
    const auto actor_e = req.thing_to_quip;
    const auto pos = get_position(r, req.thing_to_quip);

    // auto popup_e = create_popup(r, get_position(r, actor_e), "some quip");
    // r.get<WiggleUpAndDown>(popup_e).amplitude = 1.0f;

    WorldspaceTextComponent wst_c;

    wst_c.layout = [&, req](entt::registry& r, entt::entity e, const WorldspaceTextComponent& data) {
      auto* draw_list = ImGui::GetWindowDrawList();
      const auto ss_pos_tl = ImGui::GetCursorScreenPos();

      const auto text_col = hex_to_srgb("#ffffff");

      auto* font = get_inter_font(r);
      const std::string label = std::format("{}", req.message);
      draw_popup(r, ss_pos_tl, label, font, (float)FontSizes::SIZE_13, im_window_border_col);
    };

    auto popup_e = create_empty<WorldspaceTextComponent>(r, wst_c);
    r.emplace<TransformComponent>(popup_e);
    r.emplace<EntityTimedLifecycle>(popup_e, 3 * 1000);
    r.emplace<WiggleUpAndDown>(popup_e, WiggleUpAndDown{ .base_position = pos });
    set_position(r, popup_e, pos);

    //
  });
}

} // namespace game2d