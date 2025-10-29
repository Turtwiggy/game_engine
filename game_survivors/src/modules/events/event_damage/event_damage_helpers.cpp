#include "pch.hpp"

#include "event_damage_helpers.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/audio/audio_components.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/lifecycle/components.hpp"
#include "engine/maths/maths.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/actors/actor_weapon/weapon_components.hpp"
#include "modules/combat/combat_core/components.hpp"
#include "modules/combat/combat_elemental_damage/elemental_damage_components.hpp"
#include "modules/combat/combat_gun_follow_player/gun_follow_player_components.hpp"
#include "modules/combat/combat_scale_on_hit/combat_scale_on_hit_components.hpp"
#include "modules/combat/combat_weapon_type_area/combat_weapon_type_area_components.hpp"
#include "modules/combat/combat_weapon_type_projectile/combat_weapon_type_projectile_components.hpp"
#include "modules/core/animations/wiggle/components.hpp"
#include "modules/core/fonts/fonts_helpers.hpp"
#include "modules/core/ui/ui_common_components.hpp"
#include "modules/events/event_death/components.hpp"
#include "modules/events/events_core/events_components.hpp"
#include "modules/systems/system_autofire/autofire_helpers.hpp"
#include "modules/systems/system_death_throes/death_throes_components.hpp"
#include "modules/systems/system_upgrade/upgrade_components.hpp"
#include "modules/systems/system_upgrade_dodge/upgrade_dodge_components.hpp"
#include "modules/ui/ui_colours/ui_colours_helpers.hpp"
#include "modules/ui/ui_worldspace_text/components.hpp"
#include "modules/ui/ui_worldspace_text/helpers.hpp"

namespace game2d {

void
create_damage_popup(entt::registry& r, float damage, bool crit, entt::entity parent_e)
{
  WorldspaceTextComponent wst_c;

  wst_c.layout = [damage, crit](entt::registry& r, entt::entity e, const WorldspaceTextComponent& data) {
    const auto my_non_crit_col = hex_to_srgb("#b1c9c3"); // grey
    const auto my_crit_col = hex_to_srgb("#e99f10");     //

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

    const auto col = crit ? im_crit_col : im_non_crit_col;

    auto* font = get_inter_font(r);
    ImGui::PushFont(font, (float)FontSizes::SIZE_12);

    auto label = std::format("{}", (int)damage);
    if (crit)
      ImGui::TextColored(im_crit_col, "%s", label.c_str());
    else
      ImGui::TextColored(im_non_crit_col, "%s", label.c_str());

    ImGui::PopFont();
  };

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

  // damage shouldnt be negative
  return glm::max(amount, 0.0f);
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
  const auto parent_e = evt.to_parent;
  const auto fixture_e = evt.to_fixture;

  auto& dead = get_first_component<SINGLE_EntityBinComponent>(r);

  // another DamageEvent was sent but the entity is already queued to die
  const bool is_dead = std::find(dead.dead.begin(), dead.dead.end(), parent_e) != dead.dead.end();
  if (is_dead)
    return;

  // note: evt.to is a fixture, not the parent with all the components on
  auto* hp = r.try_get<HealthComponent>(fixture_e);
  if (!hp)
    return;

  // .. pop & flash the fixture
  if (const auto* t_c = r.try_get<TransformComponent>(fixture_e))
    r.emplace_or_replace<RequestHitScaleComponent>(fixture_e);
  // .. pop & flash the parent transform
  else
    r.emplace_or_replace<RequestHitScaleComponent>(parent_e);

  static engine::RandomState dodge_rnd(0);
  static engine::RandomState crit_rnd(0);
  static engine::RandomState audio_hit_rnd(0);

  if (const auto* your_stats_c = r.try_get<StatModifierComponent>(parent_e)) {
    // did you dodge?
    if (check_if_dodge(r, parent_e, dodge_rnd, *your_stats_c)) {
      create_popup(r, get_position(r, parent_e), "Dodge");
      return;
    }
  }

  // apply elemental damage ticks.
  if (evt.type == WEAPON_DAMAGE::FIRE) {
    float time = 3.0f;
    int stacks_to_apply = 1;

    if (r.all_of<WeaponComponent>(evt.from)) {

      const auto area_def = get_area_def(r, evt.from);

      // get the (modified) stacks to apply per shot.
      stacks_to_apply = area_def.stacks_per_shot;

      // get the (modified) seconds each stack lasts.
      time = area_def.stack_duration;
    }

    auto& elemental_damage_c = r.get_or_emplace<TickDamageComponent>(fixture_e);
    for (int i = 0; i < stacks_to_apply; i++)
      elemental_damage_c.fire.push_back({ WEAPON_DAMAGE::FIRE, time });
  }
  //
  else if (evt.type == WEAPON_DAMAGE::ICE) {
    const float time = 3.0f;
    auto& elemental_damage_c = r.get_or_emplace<TickDamageComponent>(fixture_e);
    elemental_damage_c.ice.push_back({ WEAPON_DAMAGE::ICE, time });
  }
  //
  else if (evt.type == WEAPON_DAMAGE::SHOCK) {
    const float time = 3.0f;
    auto& elemental_damage_c = r.get_or_emplace<TickDamageComponent>(fixture_e);
    elemental_damage_c.shock.push_back({ WEAPON_DAMAGE::SHOCK, time });
  }
  //
  else if (evt.type == WEAPON_DAMAGE::POISON) {
    const float time = 3.0f;
    auto& elemental_damage_c = r.get_or_emplace<TickDamageComponent>(fixture_e);
    elemental_damage_c.poison.push_back({ WEAPON_DAMAGE::POISON, time });
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

  // player hit audio
  if (const auto* player_c = r.try_get<const PlayerComponent>(parent_e)) {
    const auto hit_idx = engine::rand_det_s(audio_hit_rnd.rng, 1, 4);
    const auto hit_str = std::format("HIT_0{}", hit_idx);
    create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ .tag = hit_str });
  }

  create_damage_popup(r, damage, crit, parent_e);

  if (hp->hp <= 0) {

    // set dead hp at 0
    hp->hp = 0;

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
    dead.dead.push_back(parent_e);

    // const auto& parent_name = r.get<TagComponent>(parent_e).tag;
    // const auto str = std::format("{} died. Parent: {}", b_name, parent_name);
    // SDL_Log("%s", str.c_str());

    // Send death event.
    DeathEvent d_evt;
    d_evt.killed_by = evt.from; // can be entt::null
    d_evt.dead = parent_e;      // parent not fixture
    auto& evts_c = SINGLE_Events::instance;
    evts_c.dispatcher->trigger(d_evt);
    evts_c.dispatcher->update();
  }
};

} // namespace game2d