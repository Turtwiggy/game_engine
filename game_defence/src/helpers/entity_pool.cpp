#include "entity_pool.hpp"

#include "actors.hpp"
#include "entt/helpers.hpp"

namespace game2d {

void
EntityPool::update(entt::registry& r, const int desired)
{
  const int new_size = desired;
  const size_t old_size = instances.size();
  auto& dead = get_first_component<SINGLETON_EntityBinComponent>(r);

  // create
  const int to_create = new_size - old_size;
  for (int i = 0; i < to_create; i++)
    instances.push_back(create_gameplay(r, EntityType::empty_with_transform));

  // destroy
  for (auto i = old_size; i > new_size; i--) {
    const auto idx = i - 1;
    const auto entity = instances[idx];
    dead.dead.emplace(entity);
    instances.erase(instances.begin() + idx);
  }

  // now iterate instances...
};

} // namespace game2d