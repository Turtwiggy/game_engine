// header
#include "system.hpp"

#include "game/components/actors.hpp"
#include "game/modules/items/components.hpp" // hack
#include "modules/audio/helpers.hpp"
#include "modules/cursor/components.hpp"
#include "resources/audio.hpp"

// other lib headers
#include <imgui.h>

void
game2d::update_ui_editor_bar_system(GameEditor& editor, Game& game)
{
  auto& audio = editor.audio;
  auto& r = game.state;
  static bool show_free_cursor = false;

  // HACK: this does NOT belong here...
  if (r.view<WantsToSelectUnitsForItem>().size() > 0) {
    show_free_cursor = true;
    if (r.view<FreeCursorComponent>().size() == 0) {
      auto e = create_gameplay(editor, r, EntityType::free_cursor);
      create_renderable(editor, r, e, EntityType::free_cursor);
    }
  }
  if (r.view<WantsToSelectUnitsForItem>().size() == 0) {
    show_free_cursor = false;
    for (auto [entity, cursor] : r.view<FreeCursorComponent>().each())
      r.destroy(entity);
  }

  if (ImGui::BeginMainMenuBar()) {

    if (ImGui::BeginMenu("Audio")) {

      // hack: just to play some music for the moment
      // probably shouldn't live here
      if (ImGui::MenuItem("Play Music")) {
        if (audio.device != -1 && Mix_PlayingMusic() == 0) {
          Mix_PlayMusic(audio.music.begin()->data, 1); // play music
        }
      };

      // tododododo...
      ImGui::Text("(playing) 0/0:00");

      if (ImGui::Button("Load Music")) {
        // todo;
      }

      static bool mute_audio = false;
      ImGui::Text("Music muted: %i", mute_audio);
      if (ImGui::MenuItem("Toggle mute")) {
        mute_audio = !mute_audio;
        set_music_pause(mute_audio);
      }

      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Cursor")) {
      ImGui::Text("Show free cursor: %i", show_free_cursor);
      if (ImGui::Button("Toggle Free Cursor"))
        show_free_cursor = !show_free_cursor;
      ImGui::EndMenu();
    }

    // ImGui::SameLine(ImGui::GetContentRegionAvail().x);
    ImGui::Text("FPS: %0.2f", ImGui::GetIO().Framerate);
    ImGui::EndMainMenuBar();
  }
};