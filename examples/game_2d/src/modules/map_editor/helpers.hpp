#pragma once

// other lib headers
#include <entt/entt.hpp>

// c++ lib headers
#include <iostream>

namespace game2d {
//

inline void
serialize_to_text(entt::registry& registry, std::string path)
{
  std::cout << "serializing ... " << path << std::endl;

  registry.each([&registry](auto entity) {
    //
    const auto& eid = entity;
    if (eid == entt::null)
      return;
    // Serialize Entity
    // key: Entity
    // TODO: dont want incrementing ids, want uuids
    // Value: 12345678
    // std::cout << "serializing ... " << static_cast<int>(eid) << std::endl;

    // Serialize TagComponent
    if (registry.all_of<TagComponent>(eid)) {
      TagComponent& t = registry.get<TagComponent>(eid);
      std::cout << "serializing ... " << t.tag << std::endl;
    }
  });
}

void
deserialize_text_to_registry(entt::registry& registry, std::string path)
{
  std::cout << "load map from file: " << path << std::endl;
}

} // namespace game2d