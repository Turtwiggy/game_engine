// header
#include "system.hpp"

// engine
#include "engine/util_windows.hpp"

// other lib headers
#include <imgui.h>

// std lib
#include <stdio.h>
#include <string>

void
game2d::update_ui_menu_bar_system(entt::registry& registry, engine::Application& app, float dt)
{
  ImGui::Begin("Menu...");
  if (ImGui::Button("Open File")) {
    printf("open file clicked...\n");
    std::string filepath = engine::open_file(app.get_window(), "All\0*.*\0Text\0*.txt\0");
    if (!filepath.empty()) {
      printf("user wants to open file: %s \n", filepath.c_str());
    }
  }
  if (ImGui::Button("Save File")) {
    printf("save file clicked...\n");
    std::string filepath = engine::save_file(app.get_window(), "All\0*.*\0.txt\0");
    if (!filepath.empty()) {
      printf("user wants to save file: %s \n", filepath.c_str());
    }
  }
  ImGui::End();
};