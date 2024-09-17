#include "components.hpp"

#include "actors/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/sprites/components.hpp"
#include "engine/sprites/helpers.hpp"

#include <fstream>
#include <sstream>

namespace game2d {

Raws
load_raws(std::string path)
{
  // load from disk
  std::ifstream t(path);
  std::stringstream buffer;
  buffer << t.rdbuf();
  const std::string data = buffer.str();

  nlohmann::json root = nlohmann::json::parse(data);
  const auto raws = root.get<Raws>();
  return raws;
};

entt::entity
spawn_item(entt::registry& r, const std::string& key, const glm::vec2& pos)
{
  const auto& rs = get_first_component<Raws>(r);

  const auto it = std::find_if(rs.items.begin(), rs.items.end(), [&key](const Item& item) { return item.name == key; });
  if (it == rs.items.end()) {
    fmt::println("unable to find item in raw file: {}", key);
    return entt::null;
  };

  const auto idx = static_cast<int>(it - rs.items.begin());
  const Item& item_template = rs.items[idx];

  const auto e = r.create();

  r.emplace<TagComponent>(e, item_template.name);
  r.emplace<WaitForInitComponent>(e);

  // create_transform()
  {
    r.emplace<SpriteComponent>(e);
    set_sprite(r, e, item_template.renderable.sprite);
    set_colour(r, e, item_template.renderable.colour);

    TransformComponent tf;
    tf.position = { pos.x, pos.y, 0.0f };
    tf.scale = { 32, 32, 0.0f };
    r.emplace<TransformComponent>(e, tf);
    set_position(r, e, { tf.position.x, tf.position.y });
    set_size(r, e, { 32, 32 });
  }

  if (item_template.use.has_value())
    r.emplace<Use>(e);

  /*
  if (item_template.consumable.has_value()) {
    Consumable consumable_c;
    for (const auto& effect : item_template.consumable->effects) {
      if (effect == "provides_healing") {
        //
      }
    }
  }
  */

  return e;
}

} // namespace game2d