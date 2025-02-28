#include "event_damage_helpers.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/maths.hpp"
#include "engine/renderer/transform.hpp"
#include "imgui.h"
#include "modules/combat/components.hpp"
#include "modules/combat_scale_on_hit/components.hpp"
#include "modules/core_animations/wiggle/components.hpp"
#include "modules/event_coll_bullet_other/event_coll_bullet_other_components.hpp"
#include "modules/event_death/components.hpp"
#include "modules/events/events_components.hpp"
#include "modules/system_death_throes/death_throes_components.hpp"
#include "modules/system_traits/trait_components.hpp"
#include "modules/system_upgrade/upgrade_components.hpp"
#include "modules/system_upgrade_dodge/upgrade_dodge_components.hpp"
#include "modules/ui_colours/ui_colours_helpers.hpp"
#include "modules/ui_worldspace_text/components.hpp"
#include "modules/ui_worldspace_text/helpers.hpp"

#include <SDL2/SDL_log.h>
#include <glm/glm.hpp>
#include <magic_enum.hpp>

namespace game2d {

void
create_damage_popup(entt::registry& r, float damage, bool crit, entt::entity parent_e)
{
  WorldspaceTextComponent wst_c;

  wst_c.layout = [damage, crit](entt::registry& r) {
    const auto my_non_crit_col = hex_to_srgb("#b1c9c3"); // grey
    const auto my_crit_col = hex_to_srgb("#e99f10");     // orange

    ImGuiIO& io = ImGui::GetIO();
    ImGui::PushFont(io.Fonts->Fonts[3]);

    const auto im_non_crit_col = ImVec4{
      my_non_crit_col.r / 255.0f,
      my_non_crit_col.g / 255.0f,
      my_non_crit_col.b / 255.0f,
      my_non_crit_col.a / 255.0f,
    };

    const auto im_crit_col = ImVec4{
      my_crit_col.r / 255.0f,
      my_crit_col.g / 255.0f,
      my_crit_col.b / 255.0f,
      my_crit_col.a / 255.0f,
    };

    std::string label = std::format("{}", (int)damage);

    const auto ui_wh = ImGui::GetContentRegionAvail();
    const auto ui_tl = ImGui::GetCursorPos();
    const auto ui_txt_size = ImGui::CalcTextSize(label.c_str());
    ImGui::SetCursorPosX(ui_tl.x + (ui_wh.x * 0.5) - (ui_txt_size.x * 0.5));
    ImGui::SetCursorPosY(ui_tl.y + (ui_wh.y * 0.5) - (ui_txt_size.y * 0.5));

    if (crit)
      ImGui::TextColored(im_crit_col, "%s", label.c_str());
    else
      ImGui::TextColored(im_non_crit_col, "%s", label.c_str());

    ImGui::PopFont();
  };

  wst_c.flags |= ImGuiWindowFlags_NoDecoration;
  wst_c.flags |= ImGuiWindowFlags_NoFocusOnAppearing;
  wst_c.flags |= ImGuiWindowFlags_NoInputs;
  wst_c.flags |= ImGuiWindowFlags_NoNav;
  wst_c.flags |= ImGuiWindowFlags_NoBackground;
  // wst_c.alpha = 0.0f;

  auto popup_e = create_empty<WorldspaceTextComponent>(r, wst_c);
  r.emplace<TransformComponent>(popup_e);
  r.emplace<EntityTimedLifecycle>(popup_e, 1 * 3000);
  r.emplace<WiggleUpAndDown>(popup_e, WiggleUpAndDown{ .base_position = get_position(r, parent_e) });
  set_position(r, popup_e, get_position(r, parent_e));
};

float
calculate_damage_to_take(entt::registry& r, const DamageEvent& evt)
{
  const auto amount = evt.amount;
  const auto type = evt.type;
  const auto e = evt.to; // Note: evt.to is a fixture

  float amount_final = amount;

  if (type == DamageType::PHYSICAL) {
    // int defence_amount = 0;
    // if (auto* defence_c = r.try_get<DefenceComponent>(e))
    //   defence_amount = defence_c->armour;
    // amount_final -= defence_amount;
  }

  if (type == DamageType::PURE) {
    // .. pure not blocked ..
  }

  // damage shouldnt be negative
  return glm::max(amount_final, 0.0f);
};

bool
check_if_dodge(entt::registry& r, entt::entity to, engine::RandomState& rnd, const StatModifierComponent& stats_c)
{
  // praise be, to RNGesus
  const int roll = engine::rand_det_s(rnd.rng, 0, 100);

  const auto dodge_val = r.get<ActorDodgeComponent>(to).dodge_percent;
  const auto dodge_key = std::string(magic_enum::enum_name(UpgradeableStat::ACTOR_DODGE_CHANCE));
  const auto dodge_mod_val = stats_c.apply_modifiers(dodge_val, dodge_key); // percent.

  // your dodge percent is between 0 and anything
  // system produces a value between 0 and 100
  return roll < dodge_mod_val;
};

std::pair<bool, float>
check_if_crit(entt::registry& r, const DamageEvent& evt, engine::RandomState& rnd)
{
  // crit info would be on the evt.from (i.e. a bullet)
  if (evt.from == entt::null)
    return { false, 0.0 };

  const auto* bullet_crit_c = r.try_get<BulletCrit>(evt.from);
  if (!bullet_crit_c)
    return { false, 0.0 };

  // no chance to chrit
  if (bullet_crit_c->crit_chance <= 0.0f)
    return { false, 0.0 };

  //
  // note: crit values have already had modifiers
  // applied when the bullet was created
  //

  // praise be, to RNGesus
  const int roll = engine::rand_det_s(rnd.rng, 0, 100);

  const auto crit_chance_val = bullet_crit_c->crit_chance;
  const auto crit_damage_val = bullet_crit_c->crit_damage;
  const auto critical_hit = roll < crit_chance_val;
  const auto critical_mul = crit_damage_val / 100.0f; // convert percent to multiplier
  return { critical_hit, critical_mul };
};

void
handle_damage_event_take_damage(entt::registry& r, const DamageEvent& evt)
{
  const auto to_e = evt.to;
  const auto parent_e = r.get<HasParentComponent>(to_e).parent;

  // .. pop & flash the fixture
  if (const auto* t_c = r.try_get<TransformComponent>(to_e))
    r.emplace_or_replace<RequestHitScaleComponent>(to_e);
  // .. pop & flash the parent transform
  else
    r.emplace_or_replace<RequestHitScaleComponent>(parent_e);

  // note: evt.to is a fixture, not the parent with all the components on
  auto* hp = r.try_get<HealthComponent>(to_e);
  if (!hp) {
    // const auto& tag_c = r.get<TagComponent>(to_e);
    // auto err = std::format("handle_damage_event(): {} has no HealthComponent", tag_c.tag);
    // SDL_Log("%s", err.c_str());
    // throw std::runtime_error(err);
    return;
  }

  static engine::RandomState dodge_rnd(0);
  static engine::RandomState crit_rnd(0);

  const auto* your_stats_c = r.try_get<StatModifierComponent>(parent_e);
  if (your_stats_c) {
    // did you dodge?
    if (check_if_dodge(r, parent_e, dodge_rnd, *your_stats_c)) {
      create_popup(r, get_position(r, parent_e), "0");
      return;
    }
  }

  float damage = calculate_damage_to_take(r, evt);

  // did you crit?
  const auto [crit, crit_mul] = check_if_crit(r, evt, crit_rnd);
  if (crit) {
    const auto info_str = std::format("something was crit with a x{:.2f} multiplier", crit_mul);
    SDL_Log("%s", info_str.c_str());
    damage *= crit_mul;
  }

  // log evt
  // const auto b_name = std::string(r.get<TagComponent>(to_e).tag);
  // const auto message = std::format("({}) damaged for {}", b_name, damage);
  // SDL_Log("%s", message.c_str());

  // apply damage
  hp->hp -= damage;
  // SDL_Log("Something took damage");

  create_damage_popup(r, damage, crit, parent_e);

  if (hp->hp <= 0) {

    // Die now, or die soon?
    if (auto* death_throes_c = r.try_get<DeathThroesComponent>(parent_e)) {

      // keep a record of what killed you.
      if (!death_throes_c->evt_from_set) {
        death_throes_c->evt_from_set = true;
        death_throes_c->evt_from = evt.from;
      }

      // start dying!
      const auto& is_dying = r.get_or_emplace<IsDyingComponent>(parent_e);
      return;
    }
    // else: die now!
    auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);
    dead.dead.emplace(parent_e);

    // const auto& parent_name = r.get<TagComponent>(parent_e).tag;
    // const auto str = std::format("{} died. Parent: {}", b_name, parent_name);
    // SDL_Log("%s", str.c_str());

    // Send death event.
    DeathEvent d_evt;
    d_evt.killed_by = evt.from;                            // can be entt::null
    d_evt.dead = r.get<HasParentComponent>(evt.to).parent; // parent not fixture
    auto& evts = get_first_component<SINGLE_Events>(r);
    evts.dispatcher->trigger(d_evt);
    evts.dispatcher->update();
  }
};

} // namespace game2d