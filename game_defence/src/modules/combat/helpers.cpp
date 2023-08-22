#include "helpers.hpp"

#include "entt/helpers.hpp"
#include "resources/colours.hpp"
#include "sprites/components.hpp"
#include "sprites/helpers.hpp"

namespace game2d {

void
create_class(entt::registry& r, const entt::entity& e, const EntityType& type, const Weapon& weapon)
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
      r.emplace<HealthComponent>(e, 3);
      r.emplace<AttackComponent>(e, 20);
      r.emplace<RangeComponent>(e, 20);
      break;
    }
    case Weapon::shotgun: {
      r.emplace<HealthComponent>(e, 4);
      r.emplace<AttackComponent>(e, 50);
      r.emplace<RangeComponent>(e, 10);
      break;
    }
    case Weapon::grunt: {
      r.emplace<HealthComponent>(e, 3);
      r.emplace<AttackComponent>(e, 10);
      r.emplace<RangeComponent>(e, 10);
      break;
    }
  }
}

std::vector<std::string>
convert_int_to_sprites(int damage)
{
  std::string number_0 = { "NUMBER_0" };
  std::string number_1 = { "NUMBER_1" };
  std::string number_2 = { "NUMBER_2" };
  std::string number_3 = { "NUMBER_3" };
  std::string number_4 = { "NUMBER_4" };
  std::string number_5 = { "NUMBER_5" };
  std::string number_6 = { "NUMBER_6" };
  std::string number_7 = { "NUMBER_7" };
  std::string number_8 = { "NUMBER_8" };
  std::string number_9 = { "NUMBER_9" };

  std::vector<std::string> numbers;

  int number = damage;
  // this iterates over number from right to left.
  // e.g. 1230 will iterate as 0, 3, 2, 1
  while (number > 0) {
    int digit = number % 10;
    number /= 10;

    if (digit == 9)
      numbers.push_back(number_9);
    if (digit == 8)
      numbers.push_back(number_8);
    if (digit == 7)
      numbers.push_back(number_7);
    if (digit == 6)
      numbers.push_back(number_6);
    if (digit == 5)
      numbers.push_back(number_5);
    if (digit == 4)
      numbers.push_back(number_4);
    if (digit == 3)
      numbers.push_back(number_3);
    if (digit == 2)
      numbers.push_back(number_2);
    if (digit == 1)
      numbers.push_back(number_1);
    if (digit == 0)
      numbers.push_back(number_0);
  }

  std::reverse(numbers.begin(), numbers.end());
  return numbers;
}

} // namespace game2d