#include "pch.hpp"

#include "modules/ui/ui_scene_main_menu/ui_scene_main_menu_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/imgui/ui_imgui_defaults.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/core/animations/wiggle/components.hpp"
#include "modules/core/fonts/fonts_helpers.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/ui/ui_common_components.hpp"
#include "modules/core/ui/ui_common_helpers.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/steam_input/steam_input_helpers.hpp"
#include "modules/ui/ui_colours/ui_colours_helpers.hpp"
#include "modules/ui/ui_scene_header/ui_scene_header_components.hpp"
#include "modules/ui/ui_scene_main_menu/ui_scene_main_menu_components.hpp"
#include "modules/ui/ui_worldspace_text/components.hpp"

namespace game2d {
using namespace std::literals;

static glm::vec2 dropshadow_offset = { 6, 16 };
static float dropshadow_col[4] = {
  30 / 255.0f,
  50 / 255.0f,
  60 / 255.0f,
  0.5f,
};

void
text_with_dropshadow(std::string text, const ImVec4& col)
{
  const auto pos = ImGui::GetCursorPos();
  const auto dropshadow_pos = ImVec2{ pos.x + dropshadow_offset.x, pos.y + dropshadow_offset.y };

  // dropshadow first so it's below the text
  ImGui::SetCursorPos(dropshadow_pos);
  ImGui::TextColored(ImVec4(dropshadow_col[0], dropshadow_col[1], dropshadow_col[2], dropshadow_col[3]), "%s", text.c_str());

  // then the text
  ImGui::SetCursorPos(pos);
  ImGui::TextColored(col, "%s", text.c_str());
};

const auto my_greenish = hex_to_srgb("#71BBB2");
const auto im_greenish = convert_my_to_im_vec(my_greenish);

void
init_oh_buoy_header_text(entt::registry& r)
{
  GET_FIRST_OR_RETURN(SINGLE_RendererInfo, r, ri_e, ri)
  const auto viewport_size_half = ImVec2(ri.viewport_size_render_at.x * 0.5f, ri.viewport_size_render_at.y * 0.5f);

  // create a wiggly header

  // pos_x is 0 because camera is already at center
  const auto pos = glm::vec2(0, -viewport_size_half.y + ri.viewport_size_render_at.y * (2.5 / 12.0f));

  WorldspaceTextComponent wst_c;
  wst_c.text = "Oh Buoy!";
  wst_c.layout = [](entt::registry& r, const WorldspaceTextComponent& data) {
    const auto font_scale = get_first_component<SINGLE_UIScaling>(r).scaling;
    const auto font_enum = font_scale == 1.0f ? FontSize::HEADER : FontSize::HEADER_SCALED;
    auto* font = ImGui::GetIO().Fonts->Fonts[2]; // idx: 2 should be the fingerpaint header font
    ImGui::PushFont(font);

    text_with_dropshadow(data.text, im_greenish);

    ImGui::PopFont();
  };

  wst_c.flags |= ImGuiWindowFlags_NoDecoration;
  wst_c.flags |= ImGuiWindowFlags_NoInputs;
  wst_c.flags |= ImGuiWindowFlags_NoBackground;
  wst_c.flags |= ImGuiWindowFlags_NoSavedSettings;

  const auto header_e = create_empty<WorldspaceTextComponent>(r, wst_c);
  r.emplace<TransformComponent>(header_e);
  r.emplace<WiggleUpAndDown>(header_e,
                             WiggleUpAndDown{
                               .base_position = pos,
                             });
  r.emplace<MegaHeaderComponent>(header_e);

  set_position(r, header_e, pos);
};

void
update_ui_scene_main_menu(engine::SINGLE_Application& app, entt::registry& r)
{
  GET_FIRST_OR_RETURN(SINGLE_RendererInfo, r, ri_e, ri)
  GET_FIRST_OR_RETURN(SINGLE_MainMenuUI, r, ui_e, ui_c)
  GET_FIRST_OR_RETURN(SINGLE_SteamControllers, r, steam_e, steam_c)

  const auto viewport_pos = ImVec2((float)ri.viewport_pos.x, (float)ri.viewport_pos.y);
  const auto viewport_size = ImVec2((float)ri.viewport_size_render_at.x, (float)ri.viewport_size_render_at.y);
  const auto viewport_size_half = ImVec2(ri.viewport_size_render_at.x * 0.5f, ri.viewport_size_render_at.y * 0.5f);

  if (!ui_c.init) {
    init_oh_buoy_header_text(r);
    ui_c.do_init(r);
  }

  ui_c.update<RequestToShowMainMenu>(r);
  if (!ui_c.open)
    return;

  ImGuiIO& io = ImGui::GetIO();

  // button idx
  const auto font_scale = get_first_component<SINGLE_UIScaling>(r).scaling;
  const auto font_enum = font_scale == 1.0f ? FontSize::MENU_BUTTONS : FontSize::MENU_BUTTONS_SCALED;
  auto* font = get_inter_font(r, font_enum);
  ImGui::PushFont(font);

  const ImVec2 button_size = { 177.0f * font_scale, 50.0f * font_scale };
  const ImVec2 space_between_buttons = { 0, 16 * font_scale };

  ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, { 0.5f, 0.5f });
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 0.0f, 0.0f });
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 10.0f, 10.0f });
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);

  const auto pos = ImVec2(viewport_pos.x + viewport_size_half.x, viewport_pos.y + viewport_size_half.y);
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.2f));

  imgui_begin("Main Menu");
  process_input_for_ui_all_handles(r, ui_c.state);
  const auto g_input_e = get_first<InputComponent, Persistent>(r);
  const auto& g_input_c = r.get<InputComponent>(g_input_e);
  const auto& b_s = g_input_c.button_s;
  const bool do_act = std::find(b_s.begin(), b_s.end(), ActionStateEnum::DOWN) != b_s.end();

  int i = 0;
  const std::shared_ptr<Cell> root = ui_c.state.cells[0];
  std::shared_ptr<Cell> base = root;

  while (base->d != nullptr) {

    if (i > 0)
      ImGui::Dummy(space_between_buttons);

    const auto draw_button = [&](std::shared_ptr<Cell>& cell, int my_col_index) {
      auto a_def = SelectableButtonDef{
        .label = cell->name,
        .size = button_size,
        .input = do_act,
        .cell = cell,
        .active_cell = ui_c.state.active,
        .font = font,
      };
      if (selectable_button(r, a_def))
        cell->action();
    };

    draw_button(base, 0);

    if (base->r != nullptr) {
      ImGui::SameLine();
      draw_button(base->r, 1);
    }

    base = base->d;
    if (base == root)
      break;
    i++;
  }
  ImGui::PopStyleVar(5);
  ImGui::End();
  ImGui::PopFont();

  // note: could be in a separate file
  // ui_mute_sound_icon(r);
};

} // namespace game2d