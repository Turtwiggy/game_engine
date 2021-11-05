// your header
#include "modules/map_editor/system.hpp"

// components
#include "modules/map_editor/components.hpp"
#include "modules/renderer/components.hpp"

// helpers
#include "modules/map_editor/helpers.hpp"

// other lib headers
#include <imgui.h>

void
game2d::init_map_editor_system(entt::registry& registry)
{
  registry.set<SINGLETON_MapEditorInfo>(SINGLETON_MapEditorInfo());
};

void
game2d::update_map_editor_system(entt::registry& registry, engine::Application& app, float dt)
{
  SINGLETON_MapEditorInfo& i = registry.ctx<SINGLETON_MapEditorInfo>();

  ImGui::Begin("Map Editor", NULL, ImGuiWindowFlags_NoFocusOnAppearing);

  std::string map_path = "assets/2d_game/scenes/here.txt";

  if (ImGui::Button("Save map")) {
    serialize_to_text(registry, map_path);
  }

  if (ImGui::Button("Load map")) {
    entt::registry new_registry;
    deserialize_text_to_registry(new_registry, map_path);
  }

  if (ImGui::Checkbox("toggle me", &i.toggle)) {
    std::cout << "toggle swapped to: " << i.toggle << std::endl;
  }

  ImGui::End();
}
