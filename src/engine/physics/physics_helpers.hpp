#pragma once

#include "engine/physics/physics_components.hpp"

#include <entt/fwd.hpp>
#include <glm/fwd.hpp>

#include <unordered_set>
#include <utility>

namespace game2d {

struct pair_hash
{
  template<typename T1, typename T2>
  std::size_t operator()(const std::pair<T1, T2>& p) const
  {
    const auto hash1 = std::hash<T1>{}(p.first);
    const auto hash2 = std::hash<T2>{}(p.second);
    return hash1 ^ (hash2 << 1); // Combine hashes
  }
};

struct CollisionWithFixtureResult
{
  float d2 = 0.0f;
  entt::entity fixture_e = entt::null;
};

void
emplace_or_replace_physics_world(entt::registry& r);

void
physics_reset_task_count();

entt::entity
get_fixture(entt::registry& r, entt::entity e);

entt::entity
get_fixture_by_tag(entt::registry& r, entt::entity e, std::string tag);

PhysicsFixtureDef
get_fixture_def_by_tag(entt::registry& r, entt::entity e, std::string tag);

std::unordered_set<entt::entity>
get_all_in_area(entt::registry& r, b2Vec2 center, float d);

std::unordered_map<entt::entity, std::vector<CollisionWithFixtureResult>>
get_all_in_area_filtered(entt::registry& r,
                         const b2Vec2 center_in_meters,
                         const float d,
                         const std::function<bool(entt::registry&, entt::entity)>& cond);

float
meters_to_pixels(float meters);
glm::vec2
meters_to_pixels(b2Vec2 meters);

float
pixels_to_meters(float pixels);
b2Vec2
pixels_to_meters(glm::vec2 pixels);

} // namespace game2d