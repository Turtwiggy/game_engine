#include "factories.hpp"

// components
#include "components/hex.hpp"
#include "components/physics.hpp"
#include "components/player.hpp"
#include "components/rendering.hpp"

entt::entity
game2d::create_player(entt::registry& registry, glm::ivec2 pos)
{
  entt::entity r = registry.create();
  registry.emplace<Colour>(r, 1.0f, 1.0f, 1.0f, 1.0f);
  registry.emplace<Player>(r);
  registry.emplace<PositionInt>(r, pos.x, pos.y);
  registry.emplace<Size>(r, 16.0f, 16.0f);
  registry.emplace<Sprite>(r, sprite::type::PERSON_1);
  registry.emplace<Velocity>(r);
  registry.emplace<ZIndex>(r, 1);
  return r;
}