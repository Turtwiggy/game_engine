#include "pch.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "modules/controller_input_update_ui/controller_input_update_ui_helpers.hpp"
#include "modules/core_fonts/fonts_helpers.hpp"
#include "modules/core_renderer/components.hpp"
#include "modules/scene/scene_helpers.hpp"
#include "modules/system_gameover/gameover_components.hpp"
#include "modules/system_stats/stats_components.hpp"
#include "modules/ui_colours/ui_colours_helpers.hpp"
#include "modules/ui_common/ui_common_components.hpp"
#include "modules/ui_common/ui_common_helpers.hpp"
#include "ui_gameover_components.hpp"
#include "ui_gameover_system.hpp"

namespace game2d {

void
init(entt::registry& r, SINGLE_GameoverUI& ui_c)
{
  ui_c.state.rows.push_back(RowState{ .col_name = "To Menu", .action = []() {} });

  ui_c.init = true;
}

void
update_ui_gameover_system(entt::registry& r)
{
  GET_FIRST_OR_RETURN(SINGLE_RendererInfo, r, ri_e, ri_c)
  GET_FIRST_OR_RETURN(SINGLE_GameoverUI, r, ui_e, ui_c)
  GET_FIRST_OR_RETURN(SINGLE_SurviveStatsComponent, r, stats_e, stats_c);
  const auto font_scale = get_first_component<SINGLE_UIData>(r).scaling;

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

  static GameOverComponent request;
  process_requests<GameOverComponent>(r, [&ui_c](const auto& req) {
    request = req;
    ui_c.open = true;
  });

  if (!ui_c.open)
    return;

  if (!ui_c.init)
    init(r, ui_c);

  process_input_for_ui_all_handles(r, ui_c.state);
  const auto& acts = ui_c.state.actions;
  const auto do_act = std::find(acts.begin(), acts.end(), UIAction::SELECT) != acts.end();

  bool back_to_menu = false;
  const std::string discord_link = "https/discord.gg/8RTzsm25pR";
  const std::string header_win = "Congrats!";
  const std::string header_loss = "womp womp";
  const std::string subheader_w = "With some luck, you did it!";
  const std::string subheader_l = "Was it you or us? Feedback @ \n" + discord_link;

  const auto my_w_col = hex_to_srgb("#46C74F");
  const auto my_l_col = hex_to_srgb("#DF9755");
  const auto im_w_col = convert_my_to_im_vec(my_w_col);
  const auto im_l_col = convert_my_to_im_vec(my_l_col);

  const ImVec2 wh = { (float)ri_c.viewport_size_render_at.x, (float)ri_c.viewport_size_render_at.y };
  const ImVec2 tl = { 0, 0 };
  const auto ui_center = ImVec2{ tl.x + wh.x * 0.5f, tl.y + wh.y * 0.5f };

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_NoMove;
  flags |= ImGuiWindowFlags_NoBackground;
  flags |= ImGuiWindowFlags_NoFocusOnAppearing;
  flags |= ImGuiWindowFlags_NoDocking;
  flags |= ImGuiWindowFlags_NoSavedSettings;
  flags |= ImGuiWindowFlags_AlwaysAutoResize;

  ImGui::SetNextWindowPos(ui_center, ImGuiCond_Always, { 0.5f, 0.5f });

  ImGui::Begin("Gameover", NULL, flags);

  const auto ui_wh = ImGui::GetContentRegionAvail();
  const auto ui_tl = ImGui::GetCursorPos();

  // idx 2 should be fingerpaint font
  auto* fingerpaint_font = ImGui::GetIO().Fonts->Fonts[2];

  if (request.win_condition) {
    ImGui::PushFont(fingerpaint_font);
    ImGui::TextColored(im_w_col, "%s", std::format("{}", header_win).c_str());
    ImGui::PopFont();

    ImGui::Text("With some luck, you did it!");
  }
  if (!request.win_condition) {
    ImGui::PushFont(fingerpaint_font);
    ImGui::TextColored(im_l_col, "%s", std::format("{}", header_loss).c_str());
    ImGui::PopFont();

    ImGui::Text("honk!");
  }

  ImGui::NewLine();
  ImGui::Text("%s", std::format("-{} angry sea monsters", stats_c.enemies_killed).c_str());
  ImGui::Text("%s", std::format("+{} gold", stats_c.gold_earned).c_str());

  int row_idx = 0;
  int col_idx = 0;
  const ImVec2 button_size = { 88.5f * font_scale, 25.0f * font_scale };

  SelectableButtonDef def{
    .label = "To Menu",
    .size = button_size,
    .input = do_act,

    .my_row_index = 0,
    .my_col_index = 0,
    .ui_row_index = row_idx,
    .ui_col_index = col_idx,
    .ui_col_active = true,

    .font = fingerpaint_font,
  };

  ImGui::NewLine();
  ImGui::NewLine();
  ImGui::SetCursorPosX(ui_wh.x * 0.5f - button_size.x * 0.5f); // center
  if (selectable_button(r, def))
    back_to_menu = true;

  ImGui::End();

  // deletes the SINGLE_GameoverUI
  if (back_to_menu)
    move_to_scene_start(r, Scene::menu);
}

} // namespace game2d