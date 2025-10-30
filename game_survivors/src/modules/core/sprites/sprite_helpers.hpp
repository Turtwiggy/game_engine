#pragma once

#include "engine/entt/entity_pool.hpp"
#include "modules/core/renderer/components.hpp"

#include <entt/fwd.hpp>
#include <glm/fwd.hpp>

namespace game2d {

struct Sprite
{
  std::string sprite = "EMPTY";
  glm::vec2 pos{ 0, 0 };
  glm::vec2 size{ 0, 0 };
  float z_rotation = 0.0f;
  ZLayer z_idx = ZLayer::DEFAULT;
  engine::SRGBColour col{ 1.0f, 1.0f, 1.0f, 1.0f };
};

struct SINGLE_ImSprite
{
  std::vector<Sprite> sprites;

  EntityPool pool;
  int cached = 1;
};

void
begin_frame_sprite(entt::registry& r);

void
draw_sprite(entt::registry& r, const Sprite& desc);

void
draw_all_sprites(entt::registry& r);

} // namespace game2d