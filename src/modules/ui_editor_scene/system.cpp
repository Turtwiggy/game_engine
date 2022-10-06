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
  ImGui::Begin("GameEditor-Scene", NULL, ImGuiWindowFlags_NoFocusOnAppearing);

  std::string current_map = "todo";
  ImGui::Text("Map");
  ImGui::Text("Current map: %s", current_map.c_str());

  if (ImGui::Button("Load")) {
    // open_dialog.Open();
    load(r, "assets/maps/home.json");
  };

  ImGui::SameLine();
  if (ImGui::Button("Save As")) {
    // save_dialog.Open();
    save(r, "assets/maps/home.json");
  };

  ImGui::SameLine();
  if (ImGui::Button("Save")) {
    save(r, "assets/maps/home.json");
  };

  open_dialog.Display();
  save_dialog.Display();

  if (open_dialog.HasSelected()) {
    std::string full_path = open_dialog.GetSelected().string();
    std::cout << "Open filename" << full_path << "\n";
    open_dialog.ClearSelected();
  }
  if (save_dialog.HasSelected()) {
    std::string full_path = save_dialog.GetSelected().string();
    std::cout << "Save filename" << full_path << "\n";
    save_dialog.ClearSelected();
  }

  static std::vector<std::string> maps;

  ImGui::SameLine();
  if (ImGui::Button("Refresh")) {

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
    std::string relative_path = path.c_str();

    ImGui::Text("%s", relative_path.c_str());
    ImGui::SameLine();
    if (ImGui::Button("Open")) {
      std::cout << "Wants to open... " << relative_path << "\n";
    }
  }

  ImGui::Separator();

  ImGui::End();
};