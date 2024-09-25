#include "entity_pool.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/renderer/transform.hpp"

namespace game2d {

void
EntityPool::update(entt::registry& r, const int desired)
{
  const int new_size = desired;
  const int old_size = (int)instances.size();
  if (new_size == old_size)
    return;

  // create
  const int to_create = new_size - old_size;
  for (int i = 0; i < to_create; i++) {
    const auto e = r.create();
    r.emplace<TagComponent>(e, "entity-pool-entity");
    instances.push_back(e);
  }

  // destroy
  auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);
  for (int i = old_size; i > new_size; i--) {
    const auto idx = i - 1;
    const auto entity = instances[idx];
    dead.dead.emplace(entity);

    std::erase(instances, entity);
  }

  // now iterate instances...
};

} // namespace game2d