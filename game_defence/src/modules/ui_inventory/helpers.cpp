#include "helpers.hpp"

#include "actors/bags/items.hpp"
#include "actors/bags/weapons.hpp"
#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/keyboard.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"
#include "modules/ui_inventory/components.hpp"
#include "modules/ui_scene_main_menu/helpers.hpp"
#include "sprites/components.hpp"
#include "sprites/helpers.hpp"

#include "magic_enum.hpp"

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

void
item_slot_accepting_item(entt::registry& r, const entt::entity item_slot)
{
  if (ImGui::BeginDragDropTarget()) {
    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ITEM_E_PAYLOAD")) {
      IM_ASSERT(payload->DataSize == sizeof(uint32_t));
      const auto payload_eid = *(const uint32_t*)payload->Data;
      const auto payload_e = static_cast<entt::entity>(payload_eid);
      update_item_parent(r, payload_e, item_slot);
    }
    ImGui::EndDragDropTarget();
  }
};

void
item_slot_has_item(const entt::entity item_e)
{
  if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
    ImGui::SetDragDropPayload("ITEM_E_PAYLOAD", &item_e, sizeof(uint32_t));
    ImGui::EndDragDropSource();
  }
};

void
display_empty_item(entt::registry& r, entt::entity slot_e, const InventorySlotType& type, const ImVec2& size)
{
  auto& ui = get_first_component<SINGLE_UIInventoryState>(r);

  const uint32_t eid = static_cast<uint32_t>(slot_e);
  const std::string label = "inv-button##" + std::to_string(eid);

  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
  ImGui::Button(" ", size);
  ImGui::PopStyleVar();

  // sounds
  play_sound_if_hovered(r, ui.hovered_buttons, label);

  // tooltip
  const std::string label_tooltip = std::string(magic_enum::enum_name(type));
  ImGui::SetItemTooltip("%s", label_tooltip.c_str());

  // if (ImGui::BeginItemTooltip()) {
  //   ImGui::Text("I am a fancy tooltip");
  //   static float arr[] = { 0.6f, 0.1f, 1.0f, 0.5f, 0.92f, 0.1f, 0.2f };
  //   ImGui::PlotLines("Curve", arr, IM_ARRAYSIZE(arr));
  //   ImGui::Text("Sin(time) = %f", sinf((float)ImGui::GetTime()));
  //   ImGui::EndTooltip();
  // }
};

void
display_item(entt::registry& r, entt::entity slot_e, entt::entity item_e, const ImVec2& size)
{
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  const auto tex_id = search_for_texture_id_by_texture_path(ri, "monochrome")->id;
  const ImTextureID im_id = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(tex_id));
  const auto& item_tag = r.get<TagComponent>(item_e);
  auto& ui = get_first_component<SINGLE_UIInventoryState>(r);

  ImVec2 tl{ 0.0f, 0.0f };
  ImVec2 br{ 1.0f, 1.0f };

  const auto& item_c = r.get<ItemComponent>(item_e);
  const auto result = convert_sprite_to_uv(r, item_c.display_icon);
  std::tie(tl, br) = result;

  const uint32_t eid = static_cast<uint32_t>(slot_e);
  const std::string label = "inv-button##" + std::to_string(eid);

  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
  ImGui::ImageButton(label.c_str(), im_id, size, tl, br);
  ImGui::PopStyleVar();

  // sounds
  play_sound_if_hovered(r, ui.hovered_buttons, label);

  // tooltip
  ImGui::SetItemTooltip("%s", item_c.display_name.c_str());
};

void
display_inventory_slot(entt::registry& r, const entt::entity inventory_slot_e, const ImVec2& button_size, const ImVec2 pos)
{
  if (pos.x != 0)
    ImGui::SetCursorPosX(pos.x);
  if (pos.y != 0)
    ImGui::SetCursorPosY(pos.y);

  const auto& slot_c = r.get<InventorySlotComponent>(inventory_slot_e);
  const auto item_e = slot_c.item_e;

  if (item_e != entt::null)
    display_item(r, inventory_slot_e, item_e, button_size);

  if (item_e == entt::null)
    display_empty_item(r, inventory_slot_e, slot_c.type, button_size);

  // item = dragdrop source
  if (item_e != entt::null)
    item_slot_has_item(item_e);

  // no item = dragdrop target
  if (item_e == entt::null)
    item_slot_accepting_item(r, inventory_slot_e);
};

//
//
//

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

entt::entity
create_inv_armour(entt::registry& r, const entt::entity slot_e, const ArmourType& type)
{
  DataArmour data(type);

  ItemComponent item_c;
  item_c.display_icon = data.icon;
  item_c.display_name = std::string(magic_enum ::enum_name(type));
  item_c.parent_slot = slot_e;
  auto item_e = create_empty<ItemComponent>(r, item_c);
  r.emplace<ItemTypeComponent>(item_e, ItemTypeComponent{ ItemType::armour });

  // todo: each item should have it's own unique defence value?
  r.emplace<DefenceComponent>(item_e, DefenceComponent(1));

  update_item_parent(r, item_e, slot_e);
  return item_e;
};

entt::entity
create_inv_breachcharge(entt::registry& r, const entt::entity slot_e)
{
  DataBreachCharge data;

  ItemComponent item_c;
  item_c.display_icon = data.icon;
  item_c.display_name = "Breach Charge";
  item_c.parent_slot = slot_e;
  auto item_e = create_empty<ItemComponent>(r, item_c);
  r.emplace<ItemTypeComponent>(item_e, ItemTypeComponent{ ItemType::bomb });

  update_item_parent(r, item_e, slot_e);
  return item_e;
};

entt::entity
create_inv_shotgun(entt::registry& r, const entt::entity slot_e)
{
  DataWeaponShotgun data;

  ItemComponent item_c;
  item_c.display_icon = data.icon;
  item_c.display_name = "Shotgun";
  item_c.parent_slot = slot_e;
  auto item_e = create_empty<ItemComponent>(r, item_c);
  r.emplace<ItemTypeComponent>(item_e, ItemTypeComponent{ ItemType::gun });

  update_item_parent(r, item_e, slot_e);
  return item_e;
};

entt::entity
create_inv_bullets(entt::registry& r, const entt::entity slot_e, const BulletType& type)
{
  DataBullet data(type);

  ItemComponent item_c;
  item_c.display_icon = data.icon;
  item_c.display_name = std::string(magic_enum::enum_name(type));
  item_c.parent_slot = slot_e;
  auto item_e = create_empty<ItemComponent>(r, item_c);

  std::optional<ItemType> t = std::nullopt;
  if (type == BulletType::BOUNCY)
    t = ItemType::bullet_bouncy;
  if (type == BulletType::DEFAULT)
    t = ItemType::bullet_default;
  r.emplace<ItemTypeComponent>(item_e, ItemTypeComponent{ t.value() });

  update_item_parent(r, item_e, slot_e);
  return item_e;
};

} // namespace game2d