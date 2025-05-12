#include "pch.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/imgui/ui_imgui_defaults.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/core/fonts/fonts_helpers.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/ui/ui_common_components.hpp"
#include "modules/core/ui/ui_common_helpers.hpp"
#include "modules/scene/scene_helpers.hpp"
#include "modules/systems/system_gameover/gameover_components.hpp"
#include "modules/systems/system_stats/stats_components.hpp"
#include "modules/ui/ui_colours/ui_colours_helpers.hpp"
#include "resources/data.hpp"
#include "ui_gameover_components.hpp"
#include "ui_gameover_system.hpp"

namespace game2d {

void
update_ui_gameover_system(entt::registry& r)
{
  GET_FIRST_OR_RETURN(SINGLE_RendererInfo, r, ri_e, ri_c)
  GET_FIRST_OR_RETURN(SINGLE_GameoverUI, r, ui_e, ui_c)
  GET_FIRST_OR_RETURN(SINGLE_SurviveStatsComponent, r, stats_e, stats_c);
  const auto font_scale = get_first_component<SINGLE_UIScaling>(r).scaling;

#if defined(_DEBUG)
  auto input_c = get_first_component<SINGLE_InputComponent>(r);
  if (get_key_down(input_c, SDL_SCANCODE_KP_8)) {
    GameOverComponent req;
    req.win_condition = true;
    req.reason = "forced gameover (win)";
    create_empty<GameOverComponent>(r, req);
  }
  if (get_key_down(input_c, SDL_SCANCODE_KP_9)) {
    GameOverComponent req;
    req.win_condition = false;
    req.reason = "forced gameover (loss)";
    create_empty<GameOverComponent>(r, req);
  }
#endif

  const auto request_opt = ui_c.update<GameOverComponent>(r);
  if (request_opt.has_value())
    ui_c.request = request_opt.value();

  if (!ui_c.open || !ui_c.request.has_value())
    return;

  process_input_for_ui_all_handles(r, ui_c.state);
  const auto g_input_e = get_first<InputComponent, Persistent>(r);
  const auto& g_input_c = r.get<InputComponent>(g_input_e);
  const auto& b = g_input_c.button_s;
  const bool do_act = std::find(b.begin(), b.end(), ActionStateEnum::DOWN) != b.end();

  bool back_to_menu = false;
  const std::string discord_link = "https/discord.gg/8RTzsm25pR";
  const std::string header_win = "Congrats!";
  const std::string header_loss = "womp womp!";
  const std::string subheader_w = "You did it!";
  const std::string subheader_l = "Was it you or us? Feedback @ \n" + discord_link;

  const ImVec2 wh = { (float)ri_c.viewport_size_render_at.x, (float)ri_c.viewport_size_render_at.y };
  const ImVec2 tl = { 0, 0 };
  const auto ui_center = ImVec2{ tl.x + wh.x * 0.5f, tl.y + wh.y * 0.5f };

  ImGui::SetNextWindowPos(ui_center, ImGuiCond_Always, { 0.5f, 0.5f });

  static float wp = 6.0f;
  // static float wr = 0.0f;
  // static float fp = 0.0f;
  // static float fr = 0.0f;
  // imgui_draw_float("wp", wp);
  // imgui_draw_float("wr", wr);
  // imgui_draw_float("fp", fp);
  // imgui_draw_float("fr", fr);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, wp));
  // ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, wr);
  // ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(fp, fp));
  // ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, fr);

  imgui_begin("Gameover");
  const auto ui_wh = ImGui::GetContentRegionAvail();
  const auto ui_tl = ImGui::GetCursorPos();

  // idx 2 should be fingerpaint font
  auto* fingerpaint_font = ImGui::GetIO().Fonts->Fonts[2];

  const auto font_enum = font_scale == 1.0f ? FontSize::TEXT_SIZE_16 : FontSize::TEXT_SIZE_16_SCALED;
  const auto font_size = (float)font_enum;
  auto* text_font = get_inter_font(r, font_enum);

  if (ui_c.request->win_condition) {
    ImGui::PushFont(fingerpaint_font);
    ImGui::TextColored(im_w_col, "%s", std::format("{}", header_win).c_str());
    ImGui::PopFont();

    ImGui::Text("With some luck, you did it!");
  }
  if (!ui_c.request->win_condition) {
    ImGui::PushFont(fingerpaint_font);
    ImGui::TextColored(im_l_col, "%s", std::format("{}", header_loss).c_str());
    ImGui::PopFont();

    // ImGui::Text("honk!");
  }

  ImGui::NewLine();
  ImGui::Text("%s", std::format("-{} angry sea monsters", stats_c.enemies_killed).c_str());
  ImGui::Text("%s", std::format("+{} gold", stats_c.gold_earned).c_str());

  const ImVec2 button_size = { 88.5f * font_scale, 25.0f * font_scale };

  SelectableButtonDef def{
    .label = "To Menu",
    .size = button_size,
    .input = do_act,
    .cell = ui_c.state.cells[0], // only one button (continue)
    .active_cell = ui_c.state.active,
    .font = text_font,
  };

  ImGui::NewLine();
  ImGui::NewLine();
  ImGui::SetCursorPosX(ui_wh.x * 0.5f - button_size.x * 0.5f); // center
  if (selectable_button(r, def))
    back_to_menu = true;

  ImGui::End();
  ImGui::PopStyleVar();

  // deletes the SINGLE_GameoverUI
  if (back_to_menu)
    move_to_scene_start(r, Scene::menu);
}

} // namespace game2d