#include "pch.hpp"

#include "combat_helpers.hpp"

#include "modules/core/raws/raws_helpers.hpp"

namespace game2d {

SINGLE_Weapons
load_weapons(std::string filepath)
{
  SDL_Log("loading weapons... %s", filepath.c_str());

  // load from disk
  std::ifstream t(filepath);
  std::stringstream buffer;
  buffer << t.rdbuf();
  const std::string data_with_comments = buffer.str();

  // remove comments from .jsonc file
  std::istringstream stream(data_with_comments);
  std::ostringstream output;
  std::string line;
  while (std::getline(stream, line)) {
    std::string cleaned_line = remove_comment(line);
    output << cleaned_line << "\n";
  }

  const std::string string_without_comments = output.str();
  nlohmann::json root = nlohmann::json::parse(string_without_comments);

  // validate weapon type
  auto weapons_c = root.get<SINGLE_Weapons>();
  for (auto& weapon : weapons_c.weapons)
    weapon.type_as_enum = magic_enum::enum_cast<WEAPON_TYPE>(weapon.weapon_type).value();

  return weapons_c;
};

} // namespace game2d