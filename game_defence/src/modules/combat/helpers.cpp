#include "helpers.hpp"

#include "entt/helpers.hpp"
#include "resources/colours.hpp"
#include "sprites/components.hpp"
#include "sprites/helpers.hpp"

namespace game2d {

void
create_class(entt::registry& r, entt::entity& e, const EntityType& type, const Weapon& weapon)
{
  const auto& colours = get_first_component<SINGLETON_ColoursComponent>(r);
  const auto& anims = get_first_component<SINGLETON_Animations>(r);
  auto& scc = r.get<SpriteColourComponent>(e);
  auto& sc = r.get<SpriteComponent>(e);

  if (weapon == Weapon::sniper)
    scc.colour = colours.lin_blue;
  if (weapon == Weapon::shotgun)
    scc.colour = colours.lin_orange;
  if (weapon == Weapon::grunt)
    scc.colour = colours.lin_yellow;

  std::string sprite;
  if (weapon == Weapon::sniper)
    sprite = "PERSON_25_6";
  else if (weapon == Weapon::shotgun)
    sprite = "PERSON_28_1";
  else if (weapon == Weapon::grunt)
    sprite = "PERSON_25_1";

  const auto anim = find_animation(anims.animations, sprite);
  sc.x = anim.animation_frames[0].x;
  sc.y = anim.animation_frames[0].y;

  switch (weapon) {
    case Weapon::sniper: {
      r.emplace<HealthComponent>(e, 10);
      r.emplace<AttackComponent>(e, 20);
      r.emplace<RangeComponent>(e, 20);
      break;
    }
    case Weapon::shotgun: {
      r.emplace<HealthComponent>(e, 25);
      r.emplace<AttackComponent>(e, 50);
      r.emplace<RangeComponent>(e, 10);
      break;
    }
    case Weapon::grunt: {
      r.emplace<HealthComponent>(e, 10);
      r.emplace<AttackComponent>(e, 10);
      r.emplace<RangeComponent>(e, 10);
      break;
    }
  }
}

} // namespace game2d