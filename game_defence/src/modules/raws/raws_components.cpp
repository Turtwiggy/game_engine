#include "modules/raws/raws_components.hpp"

#include "actors/actor_helpers.hpp"
#include "engine/algorithm_astar_pathfinding/astar_components.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/maths.hpp"
#include "engine/physics/components.hpp"
#include "engine/physics/helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/sprites/components.hpp"
#include "engine/sprites/helpers.hpp"
#include "modules/actor_breach_charge/breach_charge_helpers.hpp"
#include "modules/colour/components.hpp"
#include "modules/combat/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/renderer/lights/components.hpp"
#include "modules/system_cooldown/components.hpp"
#include "modules/system_items_drop_on_death/helpers.hpp"
#include "modules/system_move_to_target_via_lerp/components.hpp"
#include "modules/system_names/components.hpp"
#include "modules/ui_inventory/ui_inventory_components.hpp"

#include <box2d/b2_body.h>
#include <fstream>
#include <sstream>

namespace game2d {

template<class T>
auto
find_key_or_crash(const std::vector<T>& stuff, const std::string& key)
{
  const auto it = std::find_if(stuff.begin(), stuff.end(), [&key](const T& item) { return item.name == key; });
  if (it == stuff.end()) {
    fmt::println("unable to find key in std::vector<T>: {}", key);
    exit(1); // crash
  };
  return it;
};

Raws
load_raws(std::string path)
{
  // load from disk
  std::ifstream t(path);
  std::stringstream buffer;
  buffer << t.rdbuf();
  const std::string data_with_comments = buffer.str();

  // remove comments from .jsonc file
  std::istringstream stream(data_with_comments);
  std::ostringstream output;
  std::string line;
  while (std::getline(stream, line)) {
    std::string cleaned_line = line.find("//") != std::string::npos ? "" : line;
    output << cleaned_line << "\n";
  }
  const std::string string_without_comments = output.str();

  nlohmann::json root = nlohmann::json::parse(string_without_comments);
  const auto raws = root.get<Raws>();
  return raws;
};

entt::entity
create_transform(entt::registry& r, const std::string& name)
{
  const auto e = r.create();
  r.emplace<TagComponent>(e, name);
  r.emplace<WaitForInitComponent>(e);
  r.emplace<TransformComponent>(e);
  return e;
};

entt::entity
spawn_item(entt::registry& r, const std::string& key)
{
  const auto& rs = get_first_component<Raws>(r);
  const auto it = find_key_or_crash(rs.items, key);
  const auto idx = static_cast<int>(it - rs.items.begin());
  const Item& item_template = rs.items[idx];
  float size = 32;

  // create_transform()
  const auto e = r.create();
  r.emplace<TagComponent>(e, item_template.name);
  r.emplace<WaitForInitComponent>(e);
  r.emplace<TransformComponent>(e);
  r.emplace<SpriteComponent>(e);
  set_sprite(r, e, item_template.renderable.sprite);
  set_colour(r, e, item_template.renderable.colour);
  set_z_index(r, e, ZLayer::DEFAULT);
  set_size(r, e, { size, size });

  // Add items to physics system?
  PhysicsDescription pdesc;
  pdesc.type = b2_kinematicBody;
  pdesc.size = { size, size };
  pdesc.is_sensor = true;
  create_physics_actor(r, e, pdesc);

  // item is use across the game to represent any item
  r.emplace<Item>(e, item_template);

  // item component is used by the inventory ui
  UI_ItemComponent item_c;
  item_c.display_icon = item_template.renderable.sprite;
  item_c.display_name = item_template.display_name;
  item_c.display_desc = item_template.display_desc;
  item_c.parent_slot = entt::null; // set outside this func
  r.emplace<UI_ItemComponent>(e, item_c);

  if (item_template.use.has_value())
    r.emplace<Use>(e, item_template.use.value());

  if (item_template.inventory.has_value())
    r.emplace<DefaultInventory>(e, DefaultInventory(r, item_template.inventory.value().size));

  // todo: replace with item traits
  if (key == "breach_charge") {
    add_bomb_callback(r, e);
    r.emplace<EntityTimedLifecycle>(e, 3 * 1000);
  }

  return e;
};

entt::entity
spawn_mob(entt::registry& r, const std::string& key, const glm::vec2& pos)
{
  const auto& rs = get_first_component<Raws>(r);
  const auto it = find_key_or_crash<Mob>(rs.mobs, key);
  const auto idx = static_cast<int>(it - rs.mobs.begin());
  const Mob& mob_template = rs.mobs[idx];

  const auto e = r.create();
  r.emplace<TagComponent>(e, mob_template.name);
  r.emplace<WaitForInitComponent>(e);
  r.emplace<Mob>(e, mob_template);

  float size = 32;

  // create_transform()
  {
    r.emplace<SpriteComponent>(e);
    set_sprite(r, e, mob_template.renderable.sprite);

    r.emplace<DefaultColour>(e, mob_template.renderable.colour);
    set_colour(r, e, mob_template.renderable.colour);

    TransformComponent tf;
    tf.position = { pos.x, pos.y, 0.0f };
    tf.scale = { size, size, 0.0f };
    r.emplace<TransformComponent>(e, tf);

    set_z_index(r, e, ZLayer::DEFAULT);
  }

  // create_physics()
  {
    PhysicsDescription pdesc;
    pdesc.type = b2_dynamicBody;
    pdesc.position = pos;
    pdesc.size = { size, size };
    pdesc.is_sensor = mob_template.is_sensor;
    create_physics_actor(r, e, pdesc);
  }

  // all mobs: drop_inventory_on_death()
  {
    r.emplace<DefaultBody>(e, DefaultBody(r));
    r.emplace<DefaultInventory>(e, DefaultInventory{ r, 6 * 5 });

    OnDeathCallback callback;
    callback.callback = [](entt::registry& r, const entt::entity e) {
      //
      drop_inventory_on_death_callback(r, e);
    };
    r.emplace<OnDeathCallback>(e, callback);
  }

  // r.emplace<PathfindComponent>(e, 1000); // pass through units if you must
  // r.emplace<DestroyBulletOnCollison>(e);
  // r.emplace<MoveLimitComponent>(e, 1);
  r.emplace<SpawnParticlesOnDeath>(e);
  r.emplace<HealthComponent>(e, 100, 100);
  r.emplace<DefenceComponent>(e, 0);     // should be determined by equipment
  r.emplace<PathfindComponent>(e, 1000); // pass through units if you must

  // Give each mob a random name
  static engine::RandomState rnd(0);
  const auto& names = get_first_component<SINGLE_NamesComponent>(r);
  NameComponent name_c;
  name_c.full_name = names.name[engine::rand_det_s(rnd.rng, 0, int(names.name.size()))];
  name_c.first_name = name_c.full_name.substr(0, name_c.full_name.find(' '));
  name_c.last_name = name_c.full_name.substr(name_c.full_name.find(' '), name_c.full_name.length());
  r.emplace<NameComponent>(e, name_c);

  // if (item_template.stats.){
  // }

  return e;
};

entt::entity
spawn_particle_emitter(entt::registry& r, const std::string& key, const glm::vec2& pos, const entt::entity parent)
{
  const auto e = create_transform(r, "particle_emitter");

  r.emplace<SetPositionAtDynamicTarget>(e);
  r.emplace<DynamicTargetComponent>(e, parent);

  Particle pdesc;
  pdesc.time_to_live_ms = 1 * 1000;
  pdesc.start_size = 6;
  pdesc.end_size = 2;
  if (key.find("default_explode") != std::string::npos) {
    pdesc.start_size = 16;
    pdesc.end_size = 4;
  }

  // which particle to spawn?
  ParticleEmitterComponent pedesc;
  pedesc.particle_to_emit = pdesc;
  if (key.find("default_explode") != std::string::npos) {
    pedesc.expires = true;
    pedesc.particles_to_spawn_before_emitter_expires = 10;
    pedesc.random_velocity = true;
    pedesc.spawn_all_particles_at_once = true;
  }
  r.emplace<ParticleEmitterComponent>(e, pedesc);

  // emit: particles
  CooldownComponent cooldown;
  cooldown.time_max = 0.1f;
  cooldown.time = cooldown.time_max;
  r.emplace<CooldownComponent>(e, cooldown);

  set_size(r, e, { 0, 0 }); // no size for particle emitter
  return e;
};

entt::entity
spawn_particle(entt::registry& r, const std::string& key, const Particle& desc)
{
  const auto e = create_transform(r, "particle");
  r.emplace<SpriteComponent>(e);
  set_sprite(r, e, "EMPTY");

  r.emplace<EntityTimedLifecycle>(e, desc.time_to_live_ms);
  r.emplace<VelocityComponent>(e, VelocityComponent{ desc.velocity.x, desc.velocity.y });

  // WARNING: random rotation here
  static engine::RandomState rnd(0);
  auto& t = r.get<TransformComponent>(e);
  t.rotation_radians.z = engine::rand_det_s(rnd.rng, 0.0f, 2.0f * engine::PI); // rnd rotation

  // make it shrink
  ScaleOverTimeComponent sotc;
  sotc.seconds_until_complete = desc.time_to_live_ms / 1000.0f;
  sotc.start_size = desc.start_size;
  sotc.end_size = desc.end_size;
  r.emplace<ScaleOverTimeComponent>(e, sotc);

  if (auto* col = r.try_get<DefaultColour>(e))
    set_colour(r, e, col->colour);

  set_position(r, e, desc.position);
  return e;
};

entt::entity
spawn_floor(entt::registry& r, const std::string& key, const glm::vec2& pos, const glm::vec2& size)
{
  const auto floor_e = create_transform(r, "floor");
  r.emplace<DefaultColour>(floor_e, engine::SRGBColour{ 0.5f, 0.5f, 0.5f, 1.0f });
  r.emplace<SpriteComponent>(floor_e);
  set_sprite(r, floor_e, "EMPTY");
  set_position(r, floor_e, pos);
  set_size(r, floor_e, size);
  set_colour(r, floor_e, r.get<DefaultColour>(floor_e).colour);
  set_z_index(r, floor_e, ZLayer::BACKGROUND);
  r.emplace<FloorComponent>(floor_e);
  return floor_e;
};

entt::entity
spawn_ship_part(entt::registry& r, const std::string& key)
{
  const auto& rs = get_first_component<Raws>(r);
  const auto it = find_key_or_crash(rs.ship_parts, key);
  const int idx = static_cast<int>(it - rs.ship_parts.begin());
  const ShipParts& part_template = rs.ship_parts[idx];
  float size = 32;

  auto e = r.create();
  r.emplace<TagComponent>(e, part_template.name);
  r.emplace<WaitForInitComponent>(e);
  r.emplace<ShipParts>(e, part_template);

  if (part_template.renderable.has_value()) {
    r.emplace<TransformComponent>(e);
    r.emplace<SpriteComponent>(e);
    r.emplace<DefaultColour>(e, part_template.renderable->colour);
    set_colour(r, e, part_template.renderable->colour);
    set_sprite(r, e, part_template.renderable->sprite);
    set_size(r, e, { size, size });
    set_z_index(r, e, ZLayer::DEFAULT);
  }

  // Add items to physics system?
  PhysicsDescription pdesc;
  pdesc.type = b2_staticBody;
  pdesc.size = { size, size };
  pdesc.is_sensor = true;
  create_physics_actor(r, e, pdesc);

  r.emplace<SpawnParticlesOnDeath>(e);
  r.emplace<HealthComponent>(e, 100, 100);
  r.emplace<DefenceComponent>(e, 0);     // should be determined by equipment
  r.emplace<PathfindComponent>(e, 1000); // pass through units if you must
  r.emplace<NameComponent>(e, NameComponent{ part_template.name, part_template.name, part_template.name });

  return e;
}

} // namespace game2d