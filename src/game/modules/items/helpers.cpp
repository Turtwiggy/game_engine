#include "helpers.hpp"

#include "game/components/actors.hpp"
#include "game/helpers/distance.hpp"
#include "game/modules/items/components.hpp"

#include "magic_enum.hpp"

void
game2d::use_item(GameEditor& editor, Game& game, const entt::entity& user, const entt::entity& item)
{
  auto& r = game.state;
  const auto& type = r.get<EntityTypeComponent>(item);

  // Limitation: use currently only supports:
  // "potion",
  // "scroll_damage_nearest"
  // "scroll_damage_selected_on_grid"

  std::string value_str = std::string(magic_enum::enum_name(type.type));
  std::cout << "tried to use: " << value_str << "\n";

  // USE_TYPE: SELF
  if (type.type == EntityType::potion) {
    auto& u = r.get_or_emplace<WantsToUse>(user);
    Use info;
    info.entity = item;
    info.targets = { user };
    u.items.push_back(info);
  }

  // HMM: check if ranged component?
  // if (type.type == EntityType::bolt) {
  //   auto& u = r.get_or_emplace<WantsToUse>(user);
  //   Use info;
  //   info.entity = item;
  //   // info.targets = { }; // just throw it
  //   u.items.push_back(info);
  // }

  // USE_TYPE: NEAREST
  if (type.type == EntityType::scroll_damage_nearest) {
    entt::entity nearest = get_nearest_attackable(game, user);
    auto& u = r.get_or_emplace<WantsToUse>(user);
    Use info;
    info.entity = item;
    if (nearest != entt::null)
      info.targets = { nearest };
    u.items.push_back(info);
  }

  // USE_TYPE: SELECT ENTITIES
  // if (type.type == EntityType::scroll_damage_selected_on_grid) {
  //   auto& a = r.get_or_emplace<WantsToSelectUnitsForItem>(user);
  //   Use info;
  //   info.entity = item;
  //   // info.targets; // unknown until user selects
  //   a.items.push_back(info);
  // }
}
