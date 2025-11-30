#include "pch.hpp"

#include "engine/audio/audio_components.hpp"
#include "engine/audio/helpers/sdl_mixer.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/imgui/ui_imgui_defaults.hpp"
#include "engine/physics/physics_helpers.hpp"
#include "modules/actors/actor_boat/boat_components.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/core/fonts/fonts_helpers.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/ui/ui_common_components.hpp"
#include "modules/core/ui/ui_common_helpers.hpp"
#include "modules/events/event_damage/event_damage_components.hpp"
#include "modules/events/events_core/events_components.hpp"
#include "modules/scene/scene_helpers.hpp"
#include "modules/systems/system_gameover/gameover_components.hpp"
#include "modules/systems/system_stats/stats_components.hpp"
#include "modules/ui/ui_colours/ui_colours_helpers.hpp"
#include "modules/ui/ui_scene_main_menu/helpers.hpp"
#include "modules/ui/ui_scene_main_menu_upgrades/ui_scene_upgrades_helpers.hpp"
#include "resources/data.hpp"
#include "ui_gameover_components.hpp"
#include "ui_gameover_system.hpp"

namespace game2d {

void
update_ui_gameover_system(entt::registry& r, const float dt)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  auto& ri_c = SINGLE_RendererInfo::instance;
  GET_FIRST_OR_RETURN(SINGLE_GameoverUI, r, ui_e, ui_c)
  GET_FIRST_OR_RETURN(SINGLE_SurviveStatsComponent, r, stats_e, stats_c);
  const auto font_scale = get_first_component<SINGLE_UIScaling>(r).scaling;

#if defined(_DEBUG)
  auto input_c = get_first_component<SINGLE_InputComponent>(r);
  if (get_key_down(input_c, SDL_SCANCODE_1)) {
    GameOverComponent req;
    req.win_condition = true;
    req.reason = "forced gameover (win)";
    create_empty<GameOverComponent>(r, req);
  }
  if (get_key_down(input_c, SDL_SCANCODE_2)) {
    GameOverComponent req;
    req.win_condition = false;
    req.reason = "forced gameover (loss)";
    create_empty<GameOverComponent>(r, req);
  }

  ImGui::Begin("DebugHP");
  {
    const auto& evts_c = SINGLE_Events::instance;
    auto view = r.view<PlayerBoatComponent, PlayerComponent>();
    for (const auto& [e, boat_c, player_c] : view.each()) {
      auto fixture_e = get_fixture_by_tag(r, e, "fixture_player");

      ImGui::PushID((uint32_t)e);

      auto button_str = std::format("Kill Boat {}", player_c.display_name);
      if (ImGui::Button(button_str.c_str())) {
        DamageEvent evt;
        evt.amount = 1000;
        evt.to_fixture = fixture_e;
        evt.to_parent = e;
        evts_c.dispatcher->trigger(evt);
      }

      auto damage_str = std::format("damage Boat {}", player_c.display_name);
      if (ImGui::Button(damage_str.c_str())) {
        DamageEvent evt;
        evt.amount = 2;
        evt.to_fixture = fixture_e;
        evt.to_parent = e;
        evts_c.dispatcher->trigger(evt);
      }

      ImGui::PopID();
    }
  }
  ImGui::End();

#endif

  const auto request_opt = ui_c.update<GameOverComponent>(r);
  if (request_opt.has_value()) {
    ui_c.request = request_opt.value();

    SDL_Log("Gameover!");
    audio::sdl_mixer::stop_all_audio(r); // todo: make the audio fade
    AudioRequestPlayEvent req;
    req.tag = request_opt->win_condition ? "VICTORY_0" : "DEFEAT_0";
    req.looping = true;
    create_empty<AudioRequestPlayEvent>(r, req);
  }

  if (!ui_c.open || !ui_c.request.has_value())
    return;

  // process actions.
  process_input_for_ui_all_handles(r, ui_c.state);

  const auto g_input_e = get_first<InputComponent, Persistent>(r);
  const auto& g_input_c = r.get<InputComponent>(g_input_e);
  const auto& b = g_input_c.select;
  bool hel_back = std::find(b.begin(), b.end(), ActionStateEnum::HELD) != b.end();

  // also held if mouse lmb clicked.
  hel_back |= ImGui::IsMouseDown(ImGuiMouseButton_Left);

  bool back_to_menu = false;
  const std::string header_win = "Oh Buoy!";
  const std::string header_loss = "Oh Buoy!";
  const std::string subheader_w = "You did it!";

  // update held time
  if (hel_back)
    ui_c.time_to_back += dt;
  if (!hel_back)
    ui_c.time_to_back -= dt;
  ui_c.time_to_back = glm::clamp(ui_c.time_to_back, 0.0f, ui_c.time_to_back_max);
  const bool do_act = ui_c.time_to_back >= ui_c.time_to_back_max;

  const ImVec2 window_wh = { (float)ri_c.viewport_size_render_at.x, (float)ri_c.viewport_size_render_at.y };
  const ImVec2 window_tl = { 0, 0 };
  const auto ui_center = ImVec2{ window_tl.x + window_wh.x * 0.5f, window_tl.y + window_wh.y * 0.33f };
  ImGui::SetNextWindowPos(ui_center, ImGuiCond_Always, { 0.5f, 0.5f });
  ImGui::SetNextWindowSizeConstraints({ 0, 200 }, { 1000, 1000 });

  static float wp = 6.0f;
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, wp));
  imgui_begin("Gameover");
  const auto ui_tl = ImGui::GetWindowPos();
  const auto ui_wh = ImGui::GetWindowSize();
  const auto ui_br = ImVec2{ ui_tl.x + ui_wh.x, ui_tl.y + ui_wh.y };
  auto* draw_list = ImGui::GetWindowDrawList();

  const auto col = ui_c.request->win_condition ? my_w_col : my_l_col;
  const auto im_col = ui_c.request->win_condition ? im_w_col : im_l_col;

  // background
  auto my_window_bg_col_transparent = my_window_bg_col;
  my_window_bg_col_transparent.a = 200;
  const auto im_window_bg_col_transparent = convert_my_to_im(my_window_bg_col_transparent);
  const auto rounding = 4.0f;
  const auto thickness = 2.0f;
  const auto rect_flags = ImDrawFlags_RoundCornersAll;
  draw_list->AddRectFilled(ui_tl, ui_br, im_window_bg_col_transparent, rounding);
  draw_list->AddRect(ui_tl, ui_br, im_col, rounding, rect_flags, thickness);

  const auto text_font_size = (float)FontSizes::SIZE_16;
  const auto fingerpaint_font_size = (float)FontSizes::HEADER;
  auto* text_font = get_inter_font(r);
  auto* fingerpaint_font = get_fingerpaint_font(r);

  if (ui_c.request->win_condition) {
    ImGui::PushFont(fingerpaint_font, fingerpaint_font_size);
    ImGui::TextColored(im_w_col_vec, "%s", std::format("  {}  ", header_win).c_str());
    ImGui::PopFont();

    ImGui::PushFont(text_font, text_font_size);
    ImGui::Text("  With some luck, you did it!");
    ImGui::PopFont();
  }
  if (!ui_c.request->win_condition) {
    ImGui::PushFont(fingerpaint_font, fingerpaint_font_size);
    ImGui::TextColored(im_l_col_vec, "%s", std::format("  {}  ", header_loss).c_str());
    ImGui::PopFont();

    ImGui::PushFont(text_font, text_font_size);
    ImGui::Text("  Uh Oh! You ded!");
    ImGui::PopFont();
  }

  ImGui::PushFont(text_font, text_font_size);
  ImGui::Text("  %s", std::format("{} defeated sea monsters", stats_c.enemies_killed).c_str());
  ImGui::Text("  %s", std::format("{} plundered gold", stats_c.gold_earned).c_str());
  ImGui::PopFont();

  // const ImVec2 button_size = { 88.5f * font_scale, 25.0f * font_scale };
  // SelectableButtonDef def{
  //   .display_str = "To Menu",
  //   .imgui_hash = "##tomenubutton",
  //   .size = button_size,
  //   .input = do_act,
  //   .cell = ui_c.state.cells[0], // only one button (continue)
  //   .active_cell = ui_c.state.active,
  //   .font = text_font,
  //   .font_size = text_font_size,
  // };
  // ImGui::NewLine();
  // ImGui::NewLine();
  // ImGui::SetCursorPosX(ui_wh.x * 0.5f - button_size.x * 0.5f); // center
  // if (selectable_button(r, def))
  //   back_to_menu = true;

  // draw a hold to back button.
  const auto tl = ImVec2{ ui_tl.x + 5.0f, ui_br.y - 25.0f };
  const auto br = ImVec2{ ui_br.x - 5.0f, ui_br.y - 5.0f };
  const auto percent = ui_c.time_to_back / ui_c.time_to_back_max;
  draw_purchasebar(r, tl, br, percent, "Hold (Select) for Menu", col);

  if (percent >= 1.0f)
    back_to_menu = true;

  ImGui::End();
  ImGui::PopStyleVar();

  // deletes the SINGLE_GameoverUI
  if (back_to_menu)
    move_to_scene_start(r, Scene::menu);
}

} // namespace game2d