#include "scene_select_system.hpp"

#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "scene_select_components.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/sprites/helpers.hpp"
#include "modules/core_renderer/components.hpp"
#include "modules/core_renderer/helpers.hpp"
#include "modules/core_scene/scene_helpers.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/steam_input/steam_input_helpers.hpp"
#include "modules/system_hulls/hulls_components.hpp"
#include "modules/ui_scene_main_menu_playerjoin/ui_main_menu_playerjoin_components.hpp"

#include <SDL_scancode.h>
#include <algorithm>
#include <imgui.h>
#include <stdexcept>

namespace game2d {

void
inc_or_dec_choice(int& i, int dir, const int max)
{
  i += dir;
  i = i < 0 ? max - 1 : i;
  i %= max;
};

// Display all the hulls,
// and the player can select one
void
update_ui_scene_select_system(entt::registry& r, const float dt)
{
  GET_FIRST_OR_RETURN(SINGLE_SteamControllers, r, steam_e, steam_c)
  GET_FIRST_OR_RETURN(SINGLE_SelectSceneData, r, data_e, data_c)
  GET_FIRST_OR_RETURN(SINGLE_RendererInfo, r, ri_e, ri)
  GET_FIRST_OR_RETURN(SINGLE_SteamControllerGameState, r, ui_e, ui_c)
  GET_FIRST_OR_RETURN(SINGLE_InputComponent, r, input_e, input_c)

  if (data_c.menu_to_select_scene_buffer_frame) {
    data_c.menu_to_select_scene_buffer_frame = false;
    return;
  }

  set_all_steam_controller_action_set(steam_c, ActionSet::ActionSet_GameControls);

  // auto connected = steam_c.handles
  // auto assigned = ui_c.handles
  const auto nz_handles = non_zero_handles(ui_c.handles);
  data_c.player_index_to_hull.resize(std::max((int)nz_handles.size(), 1)); // force 1 player, using keyboard at least

  // Handles == 0 means keyboard only
  // if (handles.size() == 0) {
  //   move_to_scene_start(r, Scene::menu);
  //   return;
  // }

  const auto& hulls_c = get_first_component<SINGLE_Hulls>(r);
  const int available_hulls = hulls_c.hulls.size();
  if (available_hulls == 0) {
    throw std::runtime_error("No hulls in SINGLE_Hulls()");
    exit(1); // crash
    return;
  }

  bool someone_pressed_back = false;

  for (int i = 0; i < (int)nz_handles.size(); i++) {
    //
    const auto handle = nz_handles[i];
    if (handle == 0)
      continue;

    auto& hull_choice = data_c.player_index_to_hull[i];

    if (!hull_choice.confirmed) {
      if (controller_button_down(steam_c, handle, DA::Game_Left))
        inc_or_dec_choice(hull_choice.idx, -1, available_hulls);
      if (controller_button_down(steam_c, handle, DA::Game_Right))
        inc_or_dec_choice(hull_choice.idx, 1, available_hulls);
    }

    // Confirm Ship
    if (controller_button_down(steam_c, handle, DA::Game_Select))
      hull_choice.confirmed = !hull_choice.confirmed;

    const bool you_pressed_back = controller_button_down(steam_c, handle, DA::Game_Cancel);

    // Unready the player
    if (you_pressed_back) {
      if (hull_choice.confirmed)
        hull_choice.confirmed = false;
      else
        someone_pressed_back = true;
    }
  }

  const auto& c = data_c.player_index_to_hull;
  const auto confirmed = [](const HullChoice& choice) { return choice.confirmed; };
  const auto everyone_confirmed = std::all_of(c.begin(), c.end(), confirmed);
  const auto someone_confirmed = std::any_of(c.begin(), c.end(), confirmed);
  const auto noone_confirmed = std::none_of(c.begin(), c.end(), confirmed);

  // someone_pressed_back |= get_key_down(input_c, SDL_SCANCODE_ESCAPE);
  if (someone_pressed_back && noone_confirmed) {
    move_to_scene_start(r, Scene::menu);
    return;
  }

  auto sorted_hulls = hulls_c.hulls;
  auto sort_by_hullsize = [](const ShipHullData& a, const ShipHullData& b) {
    const int size_a = a.height * a.width;
    const int size_b = b.height * b.height;
    return size_a < size_b;
  };
  std::sort(sorted_hulls.begin(), sorted_hulls.end(), sort_by_hullsize);

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_NoCollapse;
  flags |= ImGuiWindowFlags_NoTitleBar;

  const auto viewport_pos = ImVec2((float)ri.viewport_pos.x, (float)ri.viewport_pos.y);
  const auto viewport_size_half = ImVec2(ri.viewport_size_render_at.x * 0.5f, ri.viewport_size_render_at.y * 0.5f);

  const auto button_size_w = 20.0f;
  const auto button_size_half = button_size_w / 2.0f;
  const auto button_size = ImVec2{ button_size_w, button_size_w };
  const ImVec2 pivot = { 0.5f, 0.5f };
  ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, pivot);

  const auto tex_id = search_for_texture_id_by_texture_path(ri, "monochrome")->id;
  const ImTextureID im_id = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(tex_id));
  ImVec2 tl{ 0.0f, 0.0f };
  ImVec2 br{ 1.0f, 1.0f };

  for (int i = 0; i < 4; i++) {

    // Divide the screen in to (x/5), and assign positions at 1/5, 2/5, 3/5, 4/5
    const auto card_x_pos = ((1 + i) / 5.0f);
    const auto card_width = (ri.viewport_size_render_at.x / 6.0f);
    const auto card_pos = ImVec2((ri.viewport_size_render_at.x * card_x_pos), ri.viewport_size_render_at.y * 0.5f);
    ImGui::SetNextWindowPos(card_pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize({ card_width, (card_width * 9) / 16.0f });

    ImGui::Begin(("PlayerSelectCard##player"s + std::to_string(i)).c_str(), NULL, flags);
    ImGui::SeparatorText("Select...");

    const bool player_is_joined = ((int)nz_handles.size()) > i;
    ImGui::Text("Joined... %i", player_is_joined);

    const auto ui_wh = ImGui::GetContentRegionAvail();
    const auto ui_tl = ImGui::GetCursorPos();

    if (!player_is_joined) {

      const auto text_str = "Join!";
      const auto text_str_len = ImGui::CalcTextSize(text_str);
      ImGui::SetCursorPosX(ui_tl.x + (ui_wh.x * 0.5) - (text_str_len.x * 0.5));
      ImGui::SetCursorPosY(ui_tl.y + (ui_wh.y * 0.5) - (text_str_len.y * 0.5));
      ImGui::Text(text_str);

      ImGui::End();
      continue;
    }

    auto& data = data_c.player_index_to_hull[i];
    ImGui::SameLine();
    ImGui::Text("Confirmed... %i", data.confirmed);

    const int col = i;

    // flashing
    // float alpha = 0.5f + 0.5f * sinf(time * 5.0f); // Flashes between 0.5 and 1.0
    // ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, alpha)); // Red with changing alpha
    // ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.2f, 0.2f, alpha));
    // ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.0f, 0.0f, alpha));
    //
    //  ImGui::PopStyleColor(3);

    // pulsing
    // float scale = 1.0f + 0.1f * sinf(time * 5.0f); // Scale between 1.0 and 1.1
    // ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (100 - 100 * scale) * 0.5f); // Center the button
    // ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (50 - 50 * scale) * 0.5f);
    // ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10 * scale, 5 * scale));
    // if (ImGui::Button("Pulsing Button", ImVec2(100 * scale, 50 * scale)))

    // Left Arrow
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 0.2f * ui_wh.y - button_size_half);
    std::string l_label = "##left"s + std::to_string(col);
    std::tie(tl, br) = convert_sprite_to_uv(r, "ARROW_LEFT");
    if (ImGui::ImageButton(l_label.c_str(), im_id, button_size, tl, br))
      inc_or_dec_choice(data.idx, -1, available_hulls);

    // Right Arrow
    ImGui::SameLine();
    ImGui::SetCursorPosX(ui_tl.x + ui_wh.x - button_size.x);
    std::string r_label = "##right"s + std::to_string(col);
    const auto result = convert_sprite_to_uv(r, "ARROW_RIGHT");
    std::tie(tl, br) = result;
    if (ImGui::ImageButton(r_label.c_str(), im_id, button_size, tl, br))
      inc_or_dec_choice(data.idx, 1, available_hulls);

    // Player Info (centered)
    {
      ImGui::SameLine();
      ImGui::SetCursorPosX(ui_tl.x + (ui_wh.x * 0.5) - button_size_half);
      std::tie(tl, br) = convert_sprite_to_uv(r, "ICON_HEART");
      const auto my_out_col = engine::SRGBColour(0.8f, 0.8f, 0.8f, 1.0f);
      const auto im_out_col = ImVec4{ my_out_col.r / 255.0f, my_out_col.g / 255.0f, my_out_col.b / 255.0f, 0.2f };
      ImGui::Image(im_id, button_size, tl, br, im_out_col);
    }

    const auto& hull_data = sorted_hulls[data.idx];
    const auto name_str = hull_data.name;
    const auto desc_str = hull_data.desc;
    const auto name_str_w = ImGui::CalcTextSize(name_str.c_str()).x;
    const auto desc_str_w = ImGui::CalcTextSize(desc_str.c_str()).x;

    // name
    ImGui::SetCursorPosX(ui_tl.x + (ui_wh.x * 0.5) - (name_str_w * 0.5));
    ImGui::Text("%s", name_str.c_str());

    // desc
    // center aligned
    if (desc_str_w < ui_wh.x)
      ImGui::SetCursorPosX(ui_tl.x + (ui_wh.x * 0.5) - (desc_str_w * 0.5));
    // left aligned
    else
      ImGui::SetCursorPosX(ui_tl.x);
    const auto text_col = ImVec4(0.64f, 0.64f, 0.64f, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Text, text_col);
    ImGui::TextWrapped("%s", desc_str.c_str());
    ImGui::PopStyleColor(1);

    ImGui::End();
  }

  // Start a countdown..
  if (everyone_confirmed)
    data_c.countdown -= dt;
  else
    data_c.countdown = data_c.countdown_max;

  // Display timer...
  if (everyone_confirmed) {
    const auto x_pos = 0.5f;
    const auto y_pos = 0.75f;
    const auto hmm = ImVec2{ ri.viewport_size_render_at.x * x_pos, ri.viewport_size_render_at.y * y_pos };
    const auto pos = ImVec2{ ri.viewport_pos.x + hmm.x, ri.viewport_pos.y + hmm.y };
    const auto size = ImVec2{ 200, 200 };
    ImGui::SetNextWindowPos(pos, ImGuiCond_Always, { 0.5f, 0.5f });
    ImGui::SetNextWindowSize(size);

    ImGuiWindowFlags countdown_flags = 0;
    countdown_flags |= ImGuiWindowFlags_NoDecoration;
    countdown_flags |= ImGuiWindowFlags_NoCollapse;
    countdown_flags |= ImGuiWindowFlags_NoTitleBar;
    countdown_flags |= ImGuiWindowFlags_NoBackground;

    ImGuiIO& io = ImGui::GetIO();
    ImGui::PushFont(io.Fonts->Fonts[2]); // Use the larger font (index 1)
    ImGui::Begin("Countdown", NULL, countdown_flags);
    const auto ui_wh = ImGui::GetContentRegionAvail();
    const auto ui_tl = ImGui::GetCursorPos();

    const auto str = std::format("{}", ((int)data_c.countdown) + 1);
    const auto text_c_str = str.c_str();
    const auto text_c_str_len = ImGui::CalcTextSize(text_c_str);
    ImGui::SetCursorPosX(ui_tl.x + (ui_wh.x * 0.5) - (text_c_str_len.x * 0.5));
    ImGui::SetCursorPosY(ui_tl.y + (ui_wh.y * 0.5) - (text_c_str_len.y * 0.5));
    ImGui::Text("%s", text_c_str);

    ImGui::End();
    ImGui::PopFont();
  }

  // Move to next scene
  if (data_c.countdown <= 0.0f) {
    for (const auto& pc : data_c.player_index_to_hull) {
      auto hull_name = sorted_hulls[pc.idx].name;
      SelectSceneToSurviveScene data;
      // TODO: should send all the player's choices
      data.chosen_boat = hull_name;
      create_persistent<SelectSceneToSurviveScene>(r, data);
      SDL_Log("Launching survive with hull: %s", data.chosen_boat.c_str());
      move_to_scene_start(r, Scene::survive);
      break;
    }
  }

  ImGui::PopStyleVar(1);
}

} // namespace game2d