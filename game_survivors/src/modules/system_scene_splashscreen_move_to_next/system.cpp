#include "pch.hpp"

#include "system.hpp"

#include "components.hpp"
#include "engine/audio/audio_components.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "modules/core_options/options_components.hpp"
#include "modules/core_renderer/components.hpp"
#include "modules/scene/scene_helpers.hpp"

namespace game2d {
using namespace std::literals;

void
audio_warning_ui(entt::registry& r)
{
  GET_FIRST_OR_RETURN(SINGLE_RendererInfo, r, ri_e, ri)

  const auto viewport_tl = ImVec2((float)ri.viewport_pos.x, (float)ri.viewport_pos.y);
  const auto viewport_wh = ImVec2(ri.viewport_size_render_at.x, ri.viewport_size_render_at.y);
  const auto viewport_wh_half = ImVec2(viewport_wh.x * 0.5f, viewport_wh.y * 0.5f);

  const auto pos = ImVec2(viewport_tl.x + viewport_wh_half.x, viewport_tl.y + viewport_wh.y - 100);
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_AlwaysAutoResize;
  flags |= ImGuiWindowFlags_NoBackground;

  ImGui::Begin("WaitingForAudio", NULL, flags);
  ImGui::Text("Initializing Audio...");
  ImGui::Text("If your audio device changed (since launch) and this is stuck, consider restarting.");
  ImGui::End();
}

void
update_scene_splashscreen_move_to_next_system(engine::SINGLE_Application& app, entt::registry& r, const float dt)
{
  auto& data = get_first_component<SINGLE_SplashScreen>(r);
  const auto& input = get_first_component<SINGLE_InputComponent>(r);

  const auto audio_e = get_first<SINGLE_AudioComponent>(r);
  if (audio_e == entt::null)
    return; // wait for it to load

  auto& audio = get_first_component<SINGLE_AudioComponent>(r);
  if (!audio.loaded) {
    audio_warning_ui(r);
    return; // wait for it to load
  }

  // set a bunch of options settings from saved options
  const auto& options_c = get_first_component<SINGLE_GameOptions>(r);
  for (const auto& option : options_c.options)
    option->load(r);

  // After X seconds, move to menu,
  // or when audio is loaded and a key is mashed
  data.time_on_splashscreen_seconds -= dt;
  if (data.time_on_splashscreen_seconds <= 0.0f || input.unprocessed_inputs.size() > 0)
    move_to_scene_start(r, Scene::menu);
};

} // namespace game2d