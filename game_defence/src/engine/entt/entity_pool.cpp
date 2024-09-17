#include "entity_pool.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/renderer/transform.hpp"

namespace game2d {

void
EntityPool::update(entt::registry& r, const size_t desired)
{
  const size_t new_size = desired;
  const size_t old_size = instances.size();
  if (new_size == old_size)
    return;

  // create
  const size_t to_create = new_size - old_size;
  for (size_t i = 0; i < to_create; i++)
    instances.push_back(create_empty<TagComponent>(r));

  // destroy
  auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);
  for (auto i = old_size; i > new_size; i--) {
    const auto idx = i - 1;
    const auto entity = instances[idx];
    dead.dead.emplace(entity);

    std::erase(instances, entity);
  }

  // now iterate instances...
};

} // namespace game2d