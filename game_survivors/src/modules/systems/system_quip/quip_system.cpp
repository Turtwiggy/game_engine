#include "pch.hpp"

#include "quip_components.hpp"
#include "quip_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "engine/maths/maths.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/combat/combat_core/components.hpp"
#include "modules/core/animations/wiggle/components.hpp"
#include "modules/core/fonts/fonts_helpers.hpp"
#include "modules/core/ui/ui_common_helpers.hpp"
#include "modules/systems/system_island_ai/island_ai_components.hpp"
#include "modules/ui/ui_worldspace_text/components.hpp"

namespace game2d {

void
update_quip_system(entt::registry& r, const float dt)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

#if defined(_DEBUG)
  static engine::RandomState rnd_quip(0); // same roll every time
#else
  static engine::RandomState rnd_quip(engine::get_system_time_for_seed());
#endif

  // spawn a lets go message immediately, and then one every ~30 seconds
  auto& quip_system_c = gesert_component<SINGLE_QuipSystem>(r);
  quip_system_c.time_to_quip_sec_cur -= dt;
  if (quip_system_c.time_to_quip_sec_cur <= 0.0f) {
    // const auto min = 0;
    // const auto max = 5;
    const auto min = quip_system_c.time_to_quip_sec_min;
    const auto max = quip_system_c.time_to_quip_sec_max;
    const auto rnd_quip_secs = engine::rand_det_s(rnd_quip.rng, min, max);
    quip_system_c.time_to_quip_sec_cur = rnd_quip_secs;

    std::vector<entt::entity> friendly_entity;
    const auto view = r.view<const IslanderAiComponent, TeamComponent>();
    for (const auto& [e, islander_c, team_c] : view.each())
      if (team_c.team == AvailableTeams::player)
        friendly_entity.push_back(e);

    if (friendly_entity.size() > 0) {
      const auto rnd_quip_idx = engine::rand_det_s(rnd_quip.rng, 0, (int)friendly_entity.size());
      create_empty<RequestQuip>(r, RequestQuip{ .thing_to_quip = friendly_entity[rnd_quip_idx] });
    }
  }

#if defined(_DEBUG)
  const auto& input_c = get_first_component<SINGLE_InputComponent>(r);
  if (get_key_down(input_c, SDL_SCANCODE_8)) {
    const auto view = r.view<const IslanderAiComponent, TeamComponent>();
    for (const auto& [e, islander_ai_c, team_c] : view.each()) {
      if (team_c.team == AvailableTeams::enemy)
        continue;
      auto thing_to_quip = e;
      create_empty<RequestQuip>(r, RequestQuip{ .thing_to_quip = thing_to_quip });
      break;
    }
  }
#endif

  process_requests<RequestQuip>(r, [&](const RequestQuip& req) {
    const auto actor_e = req.thing_to_quip;
    const auto pos = get_position(r, req.thing_to_quip);

    // auto popup_e = create_popup(r, get_position(r, actor_e), "some quip");
    // r.get<WiggleUpAndDown>(popup_e).amplitude = 1.0f;

    WorldspaceTextComponent wst_c;
    // wst_c.show_background = true;
    // wst_c.alpha = 0.65f;
    // wst_c.offset.y = (-get_size(r, actor_e).y - size_y);

    wst_c.layout = [&](entt::registry& r, entt::entity e, const WorldspaceTextComponent& data) {
      const auto text_col = hex_to_srgb("#ffffff");

      const auto font_scale = get_first_component<SINGLE_UIScaling>(r).scaling;
      const auto font_enum = font_scale == 1.0f ? FontSize::TEXT_SMALL : FontSize::TEXT_SMALL_SCALED;
      auto* font = get_inter_font(r, font_enum);
      ImGui::PushFont(font);

      const auto im_crit_col = ImVec4{
        text_col.r / 255.0f,
        text_col.g / 255.0f,
        text_col.b / 255.0f,
        text_col.a / 255.0f,
      };

      std::string label = std::format("let's go!");

      // // center x
      // const auto& style = ImGui::GetStyle();
      // const float alignment = 0.5f;
      // const float size = ImGui::CalcTextSize(label.c_str()).x + style.FramePadding.x * 2.0f;
      // float avail = ImGui::GetContentRegionAvail().x;
      // float off = (avail - size) * alignment;
      // if (off > 0.0f)
      //   ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

      // // center y
      // const float size_y = ImGui::CalcTextSize(label.c_str()).y + style.FramePadding.y * 2.0f;
      // float avail_y = ImGui::GetContentRegionAvail().y;
      // float off_y = (avail_y - size_y) * alignment;
      // if (off_y > 0.0f)
      //   ImGui::SetCursorPosY(ImGui::GetCursorPosY() + off_y);

      ImGui::TextColored(im_crit_col, "%s", label.c_str());

      ImGui::PopFont();
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