// header
#include "system.hpp"

#include "helpers.hpp"

// other lib headers
#include <imgui.h>

#include <imfilebrowser.h>

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

static ImGui::FileBrowser open_dialog{};
static ImGui::FileBrowser save_dialog{ ImGuiFileBrowserFlags_EnterNewFilename | ImGuiFileBrowserFlags_CreateNewDir };

void
game2d::update_ui_editor_scene_system(entt::registry& r)
{
  ImGui::Begin("Editor-Scene", NULL, ImGuiWindowFlags_NoFocusOnAppearing);

  ImGui::Text("Map");
  if (ImGui::Button("Load")) {
    open_dialog.Open();
    entt::registry registry;
    load(registry, "");
  };
  ImGui::SameLine();
  if (ImGui::Button("Save")) {
    save_dialog.Open();
    entt::registry registry;
    save(registry, "");
  };

  open_dialog.Display();
  save_dialog.Display();

  if (open_dialog.HasSelected()) {
    std::cout << "Open filename" << open_dialog.GetSelected().string() << "\n";
    open_dialog.ClearSelected();
  }

  if (save_dialog.HasSelected()) {
    std::cout << "Save filename" << save_dialog.GetSelected().string() << "\n";
    save_dialog.ClearSelected();
  }

  static std::vector<std::string> maps;
  if (ImGui::Button("Refresh Maps")) {

    // configure dialogs
    open_dialog.SetTitle("Open Map");
    open_dialog.SetTypeFilters({ ".json" });
    save_dialog.SetTitle("Save Map");
    save_dialog.SetTypeFilters({ ".json" });

    maps.clear();

    std::string path = "assets/maps/";
    for (const auto& entry : std::filesystem::directory_iterator(path))
      maps.push_back(entry.path().generic_string());
  }
  ImGui::Text("Maps");
  for (const auto& path : maps) {
    ImGui::Text("%s", path.c_str());
    ImGui::SameLine();
    if (ImGui::Button("Open")) {
      std::cout << "Wants to open... " << path.c_str() << "\n";
    }
  }

  ImGui::Separator();

  ImGui::End();
};