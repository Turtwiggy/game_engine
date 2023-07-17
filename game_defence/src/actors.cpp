#include "actors.hpp"

#include "colour/colour.hpp"
#include "entt/helpers.hpp"
#include "magic_enum.hpp"
#include "renderer/components.hpp"
#include "resources/colours.hpp"
#include "resources/textures.hpp"
#include "sprites/components.hpp"
#include "sprites/helpers.hpp"

namespace game2d {

static int SPRITE_SIZE = 16;

void
to_json(json& j, const EntityTypeComponent& et)
{
  j = json{ { "type", static_cast<int>(et.type) } };
};

void
from_json(const json& j, EntityTypeComponent& et)
{
  j.at("type").get_to(et.type);
};

TransformComponent
create_transform()
{
  TransformComponent t_comp;
  t_comp.scale.x = SPRITE_SIZE;
  t_comp.scale.y = SPRITE_SIZE;
  return t_comp;
};

SpriteComponent
create_sprite(entt::registry& r, const EntityType& type)
{
  const auto type_name = std::string(magic_enum::enum_name(type));

  std::string sprite = "EMPTY";
  if (type == EntityType::empty)
    sprite = "EMPTY";
  else if (type == EntityType::actor_player)
    sprite = "PERSON_25_0";
  else
    std::cerr << "warning! sprite not implemented: " << type_name << "\n";

  RenderOrder order = RenderOrder::foreground;

  //   if (type == EntityType::tile_type_floor)
  //     order = RenderOrder::background;

  SpriteComponent sc;
  sc.render_order = order;

  auto& textures = get_first_component<SINGLETON_Textures>(r);
  auto& anims = get_first_component<SINGLETON_Animations>(r);

  // search spritesheet
  const auto anim = find_animation(anims.animations, sprite);
  sc.x = anim.animation_frames[0].x;
  sc.y = anim.animation_frames[0].y;

  // limitation: all sprites are now kenny sprites
  sc.tex_unit = get_tex_unit(textures, AvailableTexture::kenny);
  if (sc.tex_unit == get_tex_unit(textures, AvailableTexture::kenny)) {
    sc.sx = 48;
    sc.sy = 22;
  }

  return sc;
};

SpriteColourComponent
create_colour(entt::registry& r, const EntityType& type)
{
  const auto type_name = std::string(magic_enum::enum_name(type));
  const auto& colours = get_first_component<SINGLETON_ColoursComponent>(r);

  const lin_ptr& primary = colours.lin_primary;
  const lin_ptr& secondary = colours.lin_secondary;
  const lin_ptr& tertiary = colours.lin_tertiary;
  const lin_ptr& quaternary = colours.lin_quaternary;

  lin_ptr chosen_col = primary;

  SpriteColourComponent scc;
  scc.colour = chosen_col;
  return scc;
}

void
create_renderable(entt::registry& r, const entt::entity& e, const EntityType& type)
{
  r.emplace<SpriteComponent>(e, create_sprite(r, type));
  r.emplace<SpriteColourComponent>(e, create_colour(r, type));
  r.emplace<TransformComponent>(e, create_transform());
};

entt::entity
create_gameplay(entt::registry& r, const EntityType& type)
{
  const auto type_name = std::string(magic_enum::enum_name(type));

  const auto& e = r.create();
  r.emplace<TagComponent>(e, std::string(magic_enum::enum_name(type)));
  r.emplace<EntityTypeComponent>(e, type);
  create_gameplay_existing_entity(r, e, type);

  return e;
};

void
create_gameplay_existing_entity(entt::registry& r, const entt::entity& e, const EntityType& type)
{
  const auto type_name = std::string(magic_enum::enum_name(type));

  switch (type) {
    case EntityType::empty: {
      break;
    }
    case EntityType::actor_player: {
      // r.emplace<PhysicsTransformComponent>(e);
      // r.emplace<PhysicsActorComponent>(e);
      // r.emplace<GridMoveComponent>(e);
      // // gameplay
      // r.emplace<PlayerComponent>(e);
      // r.emplace<HealthComponent>(e);
      // r.emplace<TakeDamageComponent>(e);
      // r.emplace<XpComponent>(e, 0);
      // StatsComponent stats;
      // stats.con_level = 1;
      // stats.agi_level = 1;
      // stats.str_level = 1;
      // r.emplace<StatsComponent>(e, stats);
      break;
    }
    default: {
      std::cout << "warning: no gameplay implemented for: " << type_name;
    }
  }
};

} // namespace game2d