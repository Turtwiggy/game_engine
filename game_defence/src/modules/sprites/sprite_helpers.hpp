#pragma once

#include "engine/entt/entity_pool.hpp"
#include "modules/core/renderer/components.hpp"

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

struct Sprite
{
  std::string sprite = "EMPTY";
  glm::vec2 pos{ 0, 0 };
  glm::vec2 size{ 0, 0 };
  ZLayer z_idx = ZLayer::DEFAULT;
  engine::SRGBColour col{ 1.0f, 1.0f, 1.0f, 1.0f };
};

struct SINGLE_ImSprite
{
  EntityPool pool;
  int cached = 100;
  int used = 0;
};

void
begin_frame_sprite(entt::registry& r);

void
draw_sprite(entt::registry& r, const Sprite& desc);

} // namespace game2d