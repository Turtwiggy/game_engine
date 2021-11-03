// your header
#include "systems/save_map.hpp"

#include "modules/renderer/components.hpp"

// other lib headers
#include <imgui.h>

// c++ lib headers
#include <iostream>

void
game2d::update_save_map_system(entt::registry& registry, engine::Application& app, float dt)
{
  ImGui::Begin("Map Editor");

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

  ImGui::End();
}
