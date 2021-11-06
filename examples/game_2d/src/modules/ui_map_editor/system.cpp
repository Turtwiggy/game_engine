// your header
#include "modules/ui_map_editor/system.hpp"

// components
#include "modules/renderer/components.hpp"
#include "modules/ui_map_editor/components.hpp"

// helpers
#include "modules/ui_map_editor/helpers.hpp"

// other lib headers
#include <imgui.h>

void
game2d::init_ui_map_editor_system(entt::registry& registry)
{
  registry.set<SINGLETON_MapEditorInfo>(SINGLETON_MapEditorInfo());
};

void
game2d::update_ui_map_editor_system(entt::registry& registry, engine::Application& app, float dt)
{
  SINGLETON_MapEditorInfo& i = registry.ctx<SINGLETON_MapEditorInfo>();

  // temp
  std::string map_path = "assets/2d_game/scenes/here.txt";

  ImGui::Begin("Map Editor", NULL, ImGuiWindowFlags_NoFocusOnAppearing);

  if (ImGui::Button("Save map")) {
    serialize_to_text(registry, map_path);
  }

  if (ImGui::Button("Load map")) {
    entt::registry new_registry;
    deserialize_text_to_registry(new_registry, map_path);
  }

  ImGui::Checkbox("Place Sprite", &i.place_sprite);

  if (i.place_sprite) {
    auto sprite = i.sprite_to_place.value_or(sprite::type::EMPTY);
    int sprite_int = static_cast<int>(sprite);
    const int sprite_max = static_cast<int>(sprite::type::_COUNT);
    ImGui::DragInt("Sprite", &sprite_int, 1.0f, 0, sprite_max);
    i.sprite_to_place = static_cast<sprite::type>(sprite_int);
  }

  ImGui::End();
}
