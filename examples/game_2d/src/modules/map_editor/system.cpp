// your header
#include "modules/map_editor/system.hpp"

#include "modules/map_editor/components.hpp"
#include "modules/renderer/components.hpp"

// other lib headers
#include <imgui.h>

// c++ lib headers
#include <iostream>

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

  if (ImGui::Button("Save map")) {
    std::cout << "Save map" << std::endl;

    const auto& view = registry.view<const PositionIntComponent>();
    view.each([](const auto& pos) {
      //
      std::cout << "saving " << pos.x << " " << pos.y << " to file" << std::endl;
    });
  }

  if (ImGui::Button("Load map")) {
    std::cout << "load map from file" << std::endl;
  }

  if (ImGui::Checkbox("toggle me", &i.toggle)) {
    std::cout << "toggle swapped to: " << i.toggle << std::endl;
  }

  ImGui::End();
}
