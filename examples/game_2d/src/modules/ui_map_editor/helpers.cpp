#include "helpers.hpp"

// components
#include "modules/renderer/components.hpp"

void
game2d::serialize_to_text(entt::registry& registry, std::string path)
{
  std::cout << "serializing ... " << path << std::endl;

  registry.each([&registry](auto entity) {
    //
    const auto& eid = entity;
    if (eid == entt::null)
      return;
    // Serialize Entity
    // TODO: dont want incrementing ids, want uuids
    // std::cout << "serializing ... " << static_cast<int>(eid) << std::endl;

    // Serialize TagComponent... etc
    if (registry.all_of<TagComponent>(eid)) {
      TagComponent& t = registry.get<TagComponent>(eid);
      std::cout << "serializing ... " << t.tag << std::endl;
    }
  });
}

void
game2d::deserialize_text_to_registry(entt::registry& registry, std::string path)
{
  std::cout << "load map from file: " << path << std::endl;

  // destroy all entities in registry
  // load in file
  // loop over all components
  // uint64_t uuid = //
};