#include "helpers.hpp"

#include "engine/audio/audio_components.hpp"
#include "engine/audio/helpers/sdl_mixer.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/io/settings.hpp"
#include "engine/sprites/helpers.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"

#include "imgui.h"

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
    create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ "UI_HOVER_01" });
  }

  // does contain hovered but it's not hovered.
  if (item_already_hovered && !item_hovered)
    hovered_buttons.erase(it);
};

void
ui_mute_sound_icon(entt::registry& r)
{
  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);

  // show a sound icon
  ImGuiWindowFlags icon_flags = 0;
  icon_flags |= ImGuiWindowFlags_NoCollapse;
  icon_flags |= ImGuiWindowFlags_NoTitleBar;
  icon_flags |= ImGuiWindowFlags_AlwaysAutoResize;
  icon_flags |= ImGuiWindowFlags_NoBackground;

  const float distance_from_right_of_screen = 75;
  const float distance_from_top_of_screen = 75;

  const ImVec2 icon_size{ 50, 50 };
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

  // button state
  static int mute = 0;

  // set state from saved disk
  static auto disk_preference_mute = gesert_string(PLAYERPREF_MUTE, "false"s) == "true";
  static bool set_from_disk = true;
  if (set_from_disk) {
    mute = disk_preference_mute;
    set_from_disk = false;
    set_icon_state(mute);
  }

  const ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - distance_from_right_of_screen,
                                 viewport->WorkPos.y + viewport->WorkSize.y - distance_from_top_of_screen));
  ImGui::Begin("Mute Sound Icon", nullptr, icon_flags);

  // draw an audio icon
  const auto tex_id = search_for_texture_id_by_texture_path(ri, "kennynl_gameicons")->id;
  const auto im_id = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(tex_id));

  bool toggle_changed = false;
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
  if (ImGui::ImageButton("mute-icon", im_id, icon_size, tl, br)) {
    mute = mute == 1 ? 0 : 1;
    save_string(PLAYERPREF_MUTE, mute == 1 ? "true"s : "false"s);
    set_icon_state(mute);
    toggle_changed = true;
  }
  ImGui::PopStyleVar();

  auto& audio = get_first_component<SINGLE_AudioComponent>(r);
  audio.mute_all = mute;
  audio.mute_sfx = mute;

  // toggle: mute to unmute. play menu theme.
  if (toggle_changed && mute == 0) {
    SDL_Log("%s", std::format("unmute all").c_str());
    create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ "MENU_01" });
  }

  // toggle: unmute to mute. stop all music.
  if (toggle_changed && mute == 1) {
    SDL_Log("%s", std::format("muted all").c_str());
    audio::sdl_mixer::stop_all_audio(r);
  }

  ImGui::End();
};

} // namespace game2d