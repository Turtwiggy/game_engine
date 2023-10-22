#include "helpers.hpp"

#include "entt/helpers.hpp"
#include "modules/renderer/components.hpp"
#include "sprites/helpers.hpp"

namespace game2d {
using namespace std::literals;

ItemInfo
item_id_to_sprite(entt::registry& r, const int id)
{
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);

  ItemInfo info;

  // Set display info
  info.display = "EMPTY";
  if (id == 0)
    info.display = "Wine"s;
  if (id == 1)
    info.display = "Cheese"s;
  if (id == 2)
    info.display = "Gun"s;
  if (id == 3)
    info.display = "Beer"s;
  if (id == 4)
    info.display = "Meat"s;

  if (id == 0)
    info.sprite = "icon_wine"s;
  if (id == 1)
    info.sprite = "icon_cheese"s;
  if (id == 2)
    info.sprite = "icon_gun"s;
  if (id == 3)
    info.sprite = "icon_beer"s;
  if (id == 4)
    info.sprite = "icon_meat"s;

  if (id == 0)
    info.sprite_text = "text_wine"s;
  if (id == 1)
    info.sprite_text = "text_cheese"s;
  if (id == 2)
    info.sprite_text = "text_gun"s;
  if (id == 3)
    info.sprite_text = "text_beer"s;
  if (id == 4)
    info.sprite_text = "text_meat"s;

  return info;
};

}