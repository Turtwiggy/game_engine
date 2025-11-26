#include "pch.hpp"

#include "shop_helpers.hpp"

#include "modules/core/io/io_helpers.hpp"
#include "modules/core/raws/raws_helpers.hpp"
#include "shop_components.hpp"

namespace game2d {

SINGLE_Shop
load_shop(entt::registry& r, std::string filepath)
{
  SDL_Log("loading shop... %s", filepath.c_str());

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

  auto shop_c = root.get<SINGLE_Shop>();

  // save default unlocked items to disk
  for (const auto& shop_item : shop_c.items) {
    if (shop_item.default_unlocked)
      savefile_put_key(r, shop_item.key, 1);
  }
  savefile_save_disk(r);

  return shop_c;
};

} // namespace game2d