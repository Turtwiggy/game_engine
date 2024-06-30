#include "system.hpp"

#include "components.hpp"

// my libs
#include "colour/colour.hpp"
#include "entt/helpers.hpp"
#include "imgui/helpers.hpp"

// other libs
#include <imgui.h>
#include <nlohmann/json.hpp>

// std libs
#include <fmt/core.h>
#include <fstream>
#include <string>
#include <vector>

namespace game2d {
using namespace std::literals;
using json = nlohmann::json;

static std::string path = "assets/config/default_colour_palette.json";

// palette: player
// https://colorhunt.co/palette/1b262c0f4c753282b8bbe1fa
// https://colorhunt.co/palette/2121213232320d737714ffec
// https://colorhunt.co/palette/164b601b6b934fc0d0a2ff86
// https://colorhunt.co/palette/14285027496d0c7b9300a8cc

// palette: enemy
// https://colorhunt.co/palette/2d4059ea5455f07b3fffd460
// https://colorhunt.co/palette/ff9b9bffd6a5fffec4cbffa9
// https://colorhunt.co/palette/a75d5dd3756bf0997dffc3a1

// palette: neutral faction
// https://colorhunt.co/palette/00000052057b892cdcbc6ff1

// mud palette:
// https://colorhunt.co/palette/fff8ea9e7676815b5b594545

// snow palette
// https://colorhunt.co/palette/f9f7f7dbe2ef3f72af112d4e
// https://colorhunt.co/palette/f4eeffdcd6f7a6b1e1424874
// https://colorhunt.co/palette/f0f5f9c9d6df52616b1e2022
// https://colorhunt.co/palette/7579e79ab3f5a3d8f4b9fffc

// floral palette:
// https://colorhunt.co/palette/ffc8c8ff9999444f5a3e4149
// https://colorhunt.co/palette/78c1f39be8d8e2f6caf8fdcf

// dark/gloomy palette:
// https://colorhunt.co/palette/331d2c3f2e3ea78295efe1d1

// trees palette
// https://colorhunt.co/palette/edf1d69dc08b60996640513b

void
init_ui_colour_palette_system(entt::registry& r)
{
  std::ifstream f(path);
  json data = json::parse(f);

  SINGLE_ColoursInfo info;

  for (const auto& el : data["colours"]) {
    ColourTagged colour;

    const auto name = el["name"];
    colour.name = name;

    const int r = el["r"];
    const int g = el["g"];
    const int b = el["b"];
    colour.colour = { r, g, b, 1.0f };

    const auto tags = el["tags"];
    for (const auto& tag : tags)
      colour.tags.push_back(tag);

    info.colours.push_back(colour);
  }

  destroy_first_and_create<SINGLE_ColoursInfo>(r, info);
  fmt::println("colours loaded: {}", info.colours.size());
}

void
game2d::update_ui_colour_palette_system(entt::registry& r)
{
  auto& colours = get_first_component<SINGLE_ColoursInfo>(r);

  ImGui::Begin("ColourEditor");

  if (ImGui::Button("Load##colourload")) {
    // load
    std::ifstream f(path);
    json data = json::parse(f);
    // TODO
  }

  // ImGui::SameLine();
  // if (ImGui::Button("Save##coloursave")) {
  //   // save
  //   // todo: populate this properly
  //   nlohmann::json root; // create a root
  //   root["colours"] = nlohmann::json::array();
  //   for (const auto& col : colours.colours) {
  //     nlohmann::json json_col;
  //     json_col["name"] = col.name;
  //     json_col["r"] = col.colour.r;
  //     json_col["g"] = col.colour.g;
  //     json_col["b"] = col.colour.b;
  //     json_col["tags"] = nlohmann::json::array();
  //     for (const auto& tag : col.tags)
  //       json_col["tags"].push_back(tag);
  //     root["colours"].push_back(json_col);
  //   }
  // save to disk
  // disabled for the moment
  // std::string data = root.dump();
  // std::cout << data << std::endl;
  // std::ofstream fout(path);
  // fout << data.c_str();
  // }
  ImGui::Separator();

  if (ImGui::Button("Create Colour")) {
    ColourTagged new_colour;
    new_colour.name = "new colour";
    new_colour.colour = engine::SRGBColour({ 1.0f, 0.0f, 0.0f, 1.0f });
    colours.colours.push_back(new_colour);
  }

  const auto colour_widget = [](const std::string& tag, engine::SRGBColour& col) {
    float cols[4] = { col.r / 255.0f, col.g / 255.0f, col.b / 255.0f, col.a };
    if (ImGui::ColorEdit4(tag.c_str(), cols)) {
      engine::SRGBColour result;
      result.r = static_cast<int>(cols[0] * 255.0f);
      result.g = static_cast<int>(cols[1] * 255.0f);
      result.b = static_cast<int>(cols[2] * 255.0f);
      result.a = cols[3];
      col = result;
    }
  };

  for (auto i = colours.colours.size(); i > 0; i--) {
    const auto idx = i - 1;
    auto& col = colours.colours[idx];
    // ImGui::Text("Colour: %s", col.name.c_str());

    // Name
    std::string name_label = "Name##"s + std::to_string(idx);
    imgui_draw_string(name_label, col.name);

    // Delete
    ImGui::SameLine();
    std::string label = "X##colour"s + std::to_string(idx);
    if (ImGui::Button(label.c_str())) {
      // erase from back of list, while iterating backwards
      colours.colours.erase(colours.colours.begin() + idx);
    }

    // Colour
    std::string edit_label = "Colour##"s + std::to_string(idx);
    colour_widget(edit_label, col.colour);

    //
    // Tags
    //
    ImGui::Text("Tags: ");
    for (const auto& tag : col.tags) {
      ImGui::SameLine();
      ImGui::Text("%s", tag.c_str());
    }

    // Add new tag input
    // static std::string tag_label_entry = "";
    // std::string tag_label = "Tag##"s + std::to_string(idx);
    // imgui_draw_string(tag_label, tag_label_entry);

    // // Add new button
    // std::string add_tag_label = "AddTag##AddTag"s + std::to_string(idx);
    // if (ImGui::Button(add_tag_label.c_str()))
    //   col.tags.push_back(tag_label_entry);

    ImGui::Separator();
  }

  ImGui::End();
};
}