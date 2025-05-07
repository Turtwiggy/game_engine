#include "pch.hpp"

#include "helpers.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/audio/audio_components.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/sprites/helpers.hpp"
#include "modules/core/animations/wiggle/components.hpp"
#include "modules/core/fonts/fonts_helpers.hpp"
#include "modules/core/io/io_helpers.hpp"
#include "modules/core/options/options_components.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/renderer/helpers.hpp"
#include "modules/core/ui/ui_common_components.hpp"
#include "modules/ui/ui_popup_options/ui_popup_options_components.hpp"
#include "modules/ui/ui_scene_header/ui_scene_header_components.hpp"
#include "modules/ui/ui_worldspace_text/components.hpp"
#include "resources/data.hpp"

namespace game2d {
using namespace std::literals;

void
play_sound_if_hovered(entt::registry& r, std::vector<std::string>& hovered_buttons, const std::string& label)
{
  const bool item_hovered = ImGui::IsItemHovered();
  const auto it = std::find(hovered_buttons.begin(), hovered_buttons.end(), label);
  const bool item_already_hovered = it != hovered_buttons.end();

  // does not contain hovered but it's hovered.
  if (item_already_hovered && item_hovered) {

    // only one button can be hovered
    if (hovered_buttons.size() != 0)
      hovered_buttons.clear();

    hovered_buttons.push_back(label);
    create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ "UI_HOVER_0" });
  }

  // does contain hovered but it's not hovered.
  if (item_already_hovered && !item_hovered)
    hovered_buttons.erase(it);
};

void
ui_mute_sound_icon(entt::registry& r)
{
  return; // disabled

  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);
  const auto& ui_c = get_first_component<SINGLE_UIScaling>(r);
  const auto& options_c = get_first_component<SINGLE_GameOptions>(r);

  const float distance_from_right_of_screen = 75;
  const float distance_from_top_of_screen = 75;

  // show a sound icon
  ImGuiWindowFlags icon_flags = 0;
  icon_flags |= ImGuiWindowFlags_NoCollapse;
  icon_flags |= ImGuiWindowFlags_NoTitleBar;
  icon_flags |= ImGuiWindowFlags_AlwaysAutoResize;
  icon_flags |= ImGuiWindowFlags_NoBackground;
  icon_flags |= ImGuiWindowFlags_NoSavedSettings;

  ImVec2 icon_size{ 50, 50 };
  icon_size.x *= ui_c.scaling;
  icon_size.y *= ui_c.scaling;

  static ImVec2 tl{ 0.0f, 0.0f };
  static ImVec2 br{ 1.0f, 1.0f };
  const auto set_icon_state = [&](const bool muted) {
    const auto [unmute_tl, unmute_br] = convert_sprite_to_uv(r, "AUDIO"s);
    const auto [mute_tl, mute_br] = convert_sprite_to_uv(r, "AUDIO_MUTE"s);
    if (muted) {
      tl = mute_tl;
      br = mute_br;
    } else {
      tl = unmute_tl;
      br = unmute_br;
    }
  };

  // set state from saved disk
  static auto disk_pref_str = std::string(magic_enum::enum_name(GAME_OPTIONS::AUDIO_MASTER_VOLUME));
  const auto disk_mute = savefile_get_key(r, disk_pref_str);
  bool muted = false;
  if (disk_mute.has_value())
    muted = disk_mute.value().get<Audio_OnDisk>().value == 0.0f;

  const ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - distance_from_right_of_screen,
                                 viewport->WorkPos.y + viewport->WorkSize.y - distance_from_top_of_screen));

  ImGui::Begin("Mute Sound Icon", nullptr, icon_flags);

  // draw an audio icon
  const auto tex_id = search_for_texture_id_by_texture_path(ri, "kennynl_gameicons")->id;
  const auto im_id = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(tex_id));

  // bool toggle_changed = false;
  // ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

  // if (ImGui::ImageButton("mute-icon", im_id, icon_size, tl, br)) {

  //   bool new_mute = !muted;
  //   savefile_put_key(r, disk_pref_str, Audio_OnDisk{ 0.0f });
  //   //   savefile_save_disk(r);

  //   //   set_icon_state(mute);
  //   //   toggle_changed = true;
  // }

  // ImGui::PopStyleVar();

  // auto& audio = get_first_component<SINGLE_AudioComponent>(r);

  // // toggle: mute to unmute
  // if (toggle_changed && mute == 0)
  //   SDL_Log("%s", std::format("TODO: implement unmute all").c_str());

  // // toggle: unmute to mute. stop all music.
  // if (toggle_changed && mute == 1) {
  //   SDL_Log("%s", std::format("muted all").c_str());
  //   audio::sdl_mixer::stop_all_audio(r);
  // }

  // ImGui::End();
};

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
  wst_c.layout = [&ri](entt::registry& r, entt::entity e, const WorldspaceTextComponent& data) {
    const auto font_scale = get_first_component<SINGLE_UIScaling>(r).scaling;
    const auto font_enum = font_scale == 1.0f ? FontSize::HEADER : FontSize::HEADER_SCALED;
    auto* font = ImGui::GetIO().Fonts->Fonts[2]; // idx: 2 should be the fingerpaint header font
    ImGui::PushFont(font);

    text_with_dropshadow(data.text, im_greenish);

    const auto& header_c = r.get<MegaHeaderComponent>(e);
    if (header_c.icon.has_value()) {
      const float size = 64.0f;
      const auto text_size = ImGui::CalcTextSize("A");

      ImGui::SameLine();
      auto pos = ImGui::GetCursorPos();
      pos.x += 10.0f; // padding between text and icon
      pos.y += 0.5f * (text_size.y - size);
      ImGui::SetCursorPos(pos);

      const auto tex_id = search_for_texture_id_by_texture_path(ri, "custom")->id;
      const auto im_id = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(tex_id));
      const ImVec2 icon_size{ size * font_scale, size * font_scale };
      const auto [icon_tl, icon_br] = convert_sprite_to_uv(r, header_c.icon.value());
      ImGui::Image(im_id, icon_size, icon_tl, icon_br, im_greenish);
    }

    ImGui::PopFont();
  };

  wst_c.flags |= ImGuiWindowFlags_NoDecoration;
  wst_c.flags |= ImGuiWindowFlags_NoSavedSettings;
  wst_c.flags |= ImGuiWindowFlags_NoInputs;
  wst_c.flags |= ImGuiWindowFlags_NoBackground;

  const auto header_e = create_empty<WorldspaceTextComponent>(r, wst_c);
  r.emplace<TransformComponent>(header_e);
  r.emplace<WiggleUpAndDown>(header_e,
                             WiggleUpAndDown{
                               .base_position = pos,
                             });
  r.emplace<MegaHeaderComponent>(header_e);

  set_position(r, header_e, pos);
};

} // namespace game2d