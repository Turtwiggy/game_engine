#include "helpers.hpp"

#include "actors/bags/items.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/keyboard.hpp"
#include "modules/ui_inventory/components.hpp"
#include "sprites/components.hpp"
#include "sprites/helpers.hpp"

namespace game2d {

std::pair<ImVec2, ImVec2>
convert_sprite_to_uv(entt::registry& r, const std::string& sprite)
{
  // convert to imgui representation
  const auto [ss, frames] = find_animation(get_first_component<SINGLE_Animations>(r), sprite);
  const int size_x = ss.px_total;
  const int size_y = ss.py_total;
  const int cols_x = ss.nx;
  const int cols_y = ss.ny;
  const float pixels_x = size_x / float(cols_x);
  const float pixels_y = size_y / float(cols_y);
  const glm::ivec2 offset = { frames.animation_frames[0].x, frames.animation_frames[0].y };
  const ImVec2 tl = ImVec2(((offset.x * pixels_x + 0.0f) / size_x), ((offset.y * pixels_y + 0.0f) / size_y));
  const ImVec2 br = ImVec2(((offset.x * pixels_x + pixels_x) / size_x), ((offset.y * pixels_y + pixels_y) / size_y));
  return { tl, br };
};

void
toggle_inventory_display(entt::registry& r)
{
  const auto& input = get_first_component<SINGLETON_InputComponent>(r);

  if (get_key_down(input, SDL_Scancode::SDL_SCANCODE_E)) {
    // not showing => showing
    if (get_first<ShowInventoryRequest>(r) == entt::null)
      destroy_first_and_create<ShowInventoryRequest>(r);
    // showing => not showing
    else
      destroy_first<ShowInventoryRequest>(r);
  }
};

std::vector<entt::entity>
get_slots(entt::registry& r, const entt::entity e, const InventorySlotType& type)
{
  const auto& inv = r.get<DefaultInventory>(e);
  const auto& body = r.get<DefaultBody>(e);

  std::vector<entt::entity> slots;

  for (const entt::entity& e : body.body) {
    const auto& slot_c = r.get<InventorySlotComponent>(e);
    if (slot_c.type == type)
      slots.push_back(e);
  }

  for (const entt::entity& e : inv.inv) {
    const auto& slot_c = r.get<InventorySlotComponent>(e);
    if (slot_c.type == type)
      slots.push_back(e);
  }

  return slots;
};

void
update_item_parent(entt::registry& r, const entt::entity item, const entt::entity parent)
{
  auto& item_c = r.get<ItemComponent>(item);

  // move out of old parent
  r.get<InventorySlotComponent>(item_c.parent_slot).item_e = entt::null;

  // set item to new parent
  r.get<ItemComponent>(item).parent_slot = parent;

  // set parent to new child
  r.get<InventorySlotComponent>(parent).item_e = item;
};

entt::entity
create_inv_scrap(entt::registry& r, const entt::entity slot_e)
{
  DataScrap data;

  ItemComponent item_c;
  item_c.display_icon = data.icon;
  item_c.display_name = "sCrap";
  item_c.parent_slot = slot_e;
  auto item_e = create_empty<ItemComponent>(r, item_c);
  r.emplace<ItemTypeComponent>(item_e, ItemTypeComponent{ ItemType::scrap });

  update_item_parent(r, item_e, slot_e);

  return item_e;
};

} // namespace game2d