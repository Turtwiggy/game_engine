#include "modules/ui_scene_survive_upgrade/ui_survive_upgrade_system.hpp"

#include "engine/maths/maths.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/system_traits/trait_components.hpp"
#include "modules/ui_colours/ui_colours_helpers.hpp"
#include "modules/ui_common/ui_common_helpers.hpp"
#include "modules/ui_debug_menubar/ui_debug_menubar_components.hpp"

#include "engine/entt/helpers.hpp"
#include "modules/controller_input_update_ui/controller_input_update_ui_helpers.hpp"
#include "modules/core_renderer/components.hpp"
#include "modules/event_coll_player_xp/event_coll_player_xp_components.hpp"
#include "modules/events/events_components.hpp"
#include "modules/system_upgrade/upgrade_components.hpp"
#include "modules/ui_debug_menubar/ui_debug_menubar_helpers.hpp"
#include "modules/ui_scene_survive_level_up/ui_survive_level_up_components.hpp"

#include <imgui.h>
#include <magic_enum.hpp>

namespace game2d {

auto close_ui = [](SINGLE_XpComponent& sxp_c) {
  sxp_c.xp = 0;
  sxp_c.level++;
  sxp_c.xp_for_next_level += 2; // 2 harder every time
};

enum class Rarity
{
  COMMON = 1,
  UNCOMMON,
  RARE,
  LEGENDARY,
  SUPER_LEGENDARY,

  count
};

struct UpgradeRollResult
{
  Rarity rarity = Rarity::COMMON;
  UpgradeableStat upgrade;
};

struct UpgradeResultsComponent
{
  std::vector<UpgradeRollResult> results;
};

constexpr std::array<std::pair<Rarity, int>, 5> rarity_chance_map = { {
  { Rarity::COMMON, 50 },
  { Rarity::UNCOMMON, 25 },
  { Rarity::RARE, 15 },
  { Rarity::LEGENDARY, 7 },
  { Rarity::SUPER_LEGENDARY, 3 },
} };

// Calculate the sum of values at compile time
constexpr int
sum_array_values()
{
  int sum = 0;
  for (const auto& pair : rarity_chance_map)
    sum += pair.second;
  return sum;
};

#if defined(_DEBUG)
// static engine::RandomState roll_rnd(0); // same roll every time
static engine::RandomState roll_rnd(engine::get_system_time_for_seed());
#else
static engine::RandomState roll_rnd(engine::get_system_time_for_seed());
#endif

void
generate_upgrades_for_players(entt::registry& r)
{
  const std::vector<UpgradeableStat> traits_to_level_up = {
    // clang-format off
    UpgradeableStat::ACTOR_DODGE_CHANCE,
    UpgradeableStat::ACTOR_HEALTH_MAX,
    UpgradeableStat::ACTOR_HEALTH_REGEN,
    UpgradeableStat::ACTOR_SPEED,
    UpgradeableStat::ACTOR_STAMINA,
    UpgradeableStat::ACTOR_XP_ZONE_SIZE,

    // UpgradeableStat::BULLET_BOUNCE,
    UpgradeableStat::BULLET_CRIT_CHANCE,
    UpgradeableStat::BULLET_CRIT_DAMAGE,
    UpgradeableStat::BULLET_DAMAGE,
    UpgradeableStat::BULLET_KNOCKBACK,
    UpgradeableStat::BULLET_LIFESTEAL,   // %hp you recover when a bullet hits
    UpgradeableStat::BULLET_PIERCE,
    UpgradeableStat::BULLET_SIZE,
    UpgradeableStat::BULLET_SPEED,

    UpgradeableStat::WEAPON_CLIP_SIZE,
    UpgradeableStat::WEAPON_FIRERATE,
    // UpgradeableStat::WEAPON_PROJECTILES, // how many bullets to fire per shot
    // UpgradeableStat::WEAPON_SPREAD,      // at what angles
    UpgradeableStat::WEAPON_RELOAD,
    UpgradeableStat::WEAPON_RANGE
    // clang-format on
  };

  const auto view = r.view<PlayerComponent>(entt::exclude<UpgradeResultsComponent>);
  for (const auto& [e, player_c] : view.each()) {

    UpgradeResultsComponent results_c;

    // Roll 3 times for 3 upgrades.
    for (int i = 0; i < 3; i++) {
      const int roll_value = engine::rand_det_s(roll_rnd.rng, 0, (int)traits_to_level_up.size());
      const int roll_rarity = engine::rand_det_s(roll_rnd.rng, 0, 100);

      Rarity rarity = Rarity::COMMON;
      int sum = 0;
      for (auto [type, value] : rarity_chance_map) {
        sum += value;
        if (roll_rarity <= sum) {
          rarity = type;
          break;
        }
      }

      const auto upgrade = traits_to_level_up[roll_value];
      results_c.results.push_back({ .rarity = rarity, .upgrade = upgrade });
    }

    r.emplace<UpgradeResultsComponent>(e, results_c);
  }
};

const auto stat_from_stat_table = [](Rarity rarity, UpgradeableStat upgrade) -> std::pair<float, std::string> {
  float amount = 0;

  const auto rarity_str = std::string(magic_enum::enum_name(rarity));
  const auto upgrade_str = std::string(magic_enum::enum_name(upgrade));

  if (upgrade == UpgradeableStat::ACTOR_DODGE_CHANCE) {
    if (rarity == Rarity::COMMON)
      amount = 2;
    if (rarity == Rarity::UNCOMMON)
      amount = 4;
    if (rarity == Rarity::RARE)
      amount = 6;
    if (rarity == Rarity::LEGENDARY)
      amount = 8;
    if (rarity == Rarity::SUPER_LEGENDARY)
      amount = 10;
    return { amount, "stat_flat_increase" };
  }
  if (upgrade == UpgradeableStat::ACTOR_HEALTH_MAX) {
    if (rarity == Rarity::COMMON)
      amount = 10;
    if (rarity == Rarity::UNCOMMON)
      amount = 20;
    if (rarity == Rarity::RARE)
      amount = 30;
    if (rarity == Rarity::LEGENDARY)
      amount = 40;
    if (rarity == Rarity::SUPER_LEGENDARY)
      amount = 50;
    return { amount, "stat_flat_increase" };
  }
  if (upgrade == UpgradeableStat::ACTOR_HEALTH_REGEN) {
    if (rarity == Rarity::COMMON)
      amount = 0.03;
    if (rarity == Rarity::UNCOMMON)
      amount = 0.1;
    if (rarity == Rarity::RARE)
      amount = 0.15;
    if (rarity == Rarity::LEGENDARY)
      amount = 0.25;
    if (rarity == Rarity::SUPER_LEGENDARY)
      amount = 0.4;
    return { amount, "stat_flat_increase" };
  }
  if (upgrade == UpgradeableStat::ACTOR_SPEED) {
    if (rarity == Rarity::COMMON)
      amount = 5;
    if (rarity == Rarity::UNCOMMON)
      amount = 10;
    if (rarity == Rarity::RARE)
      amount = 20;
    if (rarity == Rarity::LEGENDARY)
      amount = 35;
    if (rarity == Rarity::SUPER_LEGENDARY)
      amount = 50;
    return { amount, "stat_percent_increase" };
  }
  if (upgrade == UpgradeableStat::ACTOR_STAMINA) {
    if (rarity == Rarity::COMMON)
      amount = 1;
    if (rarity == Rarity::UNCOMMON)
      amount = 2;
    if (rarity == Rarity::RARE)
      amount = 3;
    if (rarity == Rarity::LEGENDARY)
      amount = 4;
    if (rarity == Rarity::SUPER_LEGENDARY)
      amount = 5;
    return { amount, "stat_flat_increase" };
  }
  if (upgrade == UpgradeableStat::ACTOR_XP_ZONE_SIZE) {
    if (rarity == Rarity::COMMON)
      amount = 5;
    if (rarity == Rarity::UNCOMMON)
      amount = 15;
    if (rarity == Rarity::RARE)
      amount = 25;
    if (rarity == Rarity::LEGENDARY)
      amount = 35;
    if (rarity == Rarity::SUPER_LEGENDARY)
      amount = 45;
    return { amount, "stat_percent_increase" };
  }

  if (upgrade == UpgradeableStat::BULLET_CRIT_CHANCE) {
    if (rarity == Rarity::COMMON)
      amount = 3;
    if (rarity == Rarity::UNCOMMON)
      amount = 6;
    if (rarity == Rarity::RARE)
      amount = 9;
    if (rarity == Rarity::LEGENDARY)
      amount = 12;
    if (rarity == Rarity::SUPER_LEGENDARY)
      amount = 15;
    return { amount, "stat_flat_increase" };
  }
  if (upgrade == UpgradeableStat::BULLET_CRIT_DAMAGE) {
    if (rarity == Rarity::COMMON)
      amount = 10;
    if (rarity == Rarity::UNCOMMON)
      amount = 20;
    if (rarity == Rarity::RARE)
      amount = 50;
    if (rarity == Rarity::LEGENDARY)
      amount = 75;
    if (rarity == Rarity::SUPER_LEGENDARY)
      amount = 100;
    return { amount, "stat_flat_increase" };
  }
  if (upgrade == UpgradeableStat::BULLET_DAMAGE) {
    if (rarity == Rarity::COMMON)
      amount = 10;
    if (rarity == Rarity::UNCOMMON)
      amount = 20;
    if (rarity == Rarity::RARE)
      amount = 30;
    if (rarity == Rarity::LEGENDARY)
      amount = 40;
    if (rarity == Rarity::SUPER_LEGENDARY)
      amount = 50;
    return { amount, "stat_percent_increase" };
  }
  if (upgrade == UpgradeableStat::BULLET_KNOCKBACK) {
    if (rarity == Rarity::COMMON)
      amount = 0.1;
    if (rarity == Rarity::UNCOMMON)
      amount = 0.2;
    if (rarity == Rarity::RARE)
      amount = 0.3;
    if (rarity == Rarity::LEGENDARY)
      amount = 0.4;
    if (rarity == Rarity::SUPER_LEGENDARY)
      amount = 0.5;
    return { amount, "stat_flat_increase" };
  }
  if (upgrade == UpgradeableStat::BULLET_LIFESTEAL) {
    if (rarity == Rarity::COMMON)
      amount = 1;
    if (rarity == Rarity::UNCOMMON)
      amount = 2;
    if (rarity == Rarity::RARE)
      amount = 3;
    if (rarity == Rarity::LEGENDARY)
      amount = 4;
    if (rarity == Rarity::SUPER_LEGENDARY)
      amount = 5;
    return { amount, "stat_flat_increase" };
  }
  if (upgrade == UpgradeableStat::BULLET_PIERCE) {
    if (rarity == Rarity::COMMON)
      amount = 1;
    if (rarity == Rarity::UNCOMMON)
      amount = 2;
    if (rarity == Rarity::RARE)
      amount = 3;
    if (rarity == Rarity::LEGENDARY)
      amount = 4;
    if (rarity == Rarity::SUPER_LEGENDARY)
      amount = 5;
    return { amount, "stat_flat_increase" };
  }
  if (upgrade == UpgradeableStat::BULLET_SIZE) {
    if (rarity == Rarity::COMMON)
      amount = 10;
    if (rarity == Rarity::UNCOMMON)
      amount = 20;
    if (rarity == Rarity::RARE)
      amount = 30;
    if (rarity == Rarity::LEGENDARY)
      amount = 40;
    if (rarity == Rarity::SUPER_LEGENDARY)
      amount = 50;
    return { amount, "stat_percent_increase" };
  }
  if (upgrade == UpgradeableStat::BULLET_SPEED) {
    if (rarity == Rarity::COMMON)
      amount = 10;
    if (rarity == Rarity::UNCOMMON)
      amount = 20;
    if (rarity == Rarity::RARE)
      amount = 30;
    if (rarity == Rarity::LEGENDARY)
      amount = 40;
    if (rarity == Rarity::SUPER_LEGENDARY)
      amount = 50;
    return { amount, "stat_percent_increase" };
  }

  if (upgrade == UpgradeableStat::WEAPON_CLIP_SIZE) {
    if (rarity == Rarity::COMMON)
      amount = 10;
    if (rarity == Rarity::UNCOMMON)
      amount = 20;
    if (rarity == Rarity::RARE)
      amount = 30;
    if (rarity == Rarity::LEGENDARY)
      amount = 40;
    if (rarity == Rarity::SUPER_LEGENDARY)
      amount = 50;
    return { amount, "stat_percent_increase" };
  }
  if (upgrade == UpgradeableStat::WEAPON_FIRERATE) {
    if (rarity == Rarity::COMMON)
      amount = 10;
    if (rarity == Rarity::UNCOMMON)
      amount = 15;
    if (rarity == Rarity::RARE)
      amount = 25;
    if (rarity == Rarity::LEGENDARY)
      amount = 35;
    if (rarity == Rarity::SUPER_LEGENDARY)
      amount = 50;
    return { amount, "stat_percent_increase" };
  }
  if (upgrade == UpgradeableStat::WEAPON_RANGE) {
    if (rarity == Rarity::COMMON)
      amount = 10;
    if (rarity == Rarity::UNCOMMON)
      amount = 20;
    if (rarity == Rarity::RARE)
      amount = 30;
    if (rarity == Rarity::LEGENDARY)
      amount = 40;
    if (rarity == Rarity::SUPER_LEGENDARY)
      amount = 50;
    return { amount, "stat_percent_increase" };
  }
  if (upgrade == UpgradeableStat::WEAPON_RELOAD) {
    if (rarity == Rarity::COMMON)
      amount = -10;
    if (rarity == Rarity::UNCOMMON)
      amount = -20;
    if (rarity == Rarity::RARE)
      amount = -30;
    if (rarity == Rarity::LEGENDARY)
      amount = -40;
    if (rarity == Rarity::SUPER_LEGENDARY)
      amount = -50;
    return { amount, "stat_percent_increase" };
  }

  const auto err_str = std::format("Not impl: {}, {}", rarity_str, upgrade_str);
  throw std::runtime_error(err_str.c_str());
};

void
update_ui_survive_upgrade_system(entt::registry& r)
{
  GET_FIRST_OR_RETURN(SINGLE_XpComponent, r, sxp_e, sxp_c);
  GET_FIRST_OR_RETURN(SINGLE_RendererInfo, r, ri_e, ri);
  GET_FIRST_OR_RETURN(SINGLE_LevelUpUI, r, ui_e, ui_c);
  GET_FIRST_OR_RETURN(SINGLE_Upgrades, r, up_e, up_c);
  GET_FIRST_OR_RETURN(SINGLE_Events, r, evts_e, evts_c)

  const bool level_up_required = sxp_c.xp >= sxp_c.xp_for_next_level;

  // Cheats..!! CHEATSS!!! CHEEEATTTSSSSSSS!!!!!!!
  {
    auto& menu_c = get_first_component<SINGLE_DebugMenuBar>(r);
    auto cheat_levelup_state = gesert_menubar_state(menu_c, "Cheat LevelUp");
    if (cheat_levelup_state.enabled) {
      ImGui::Begin("CheatLevelUp");
      if (ImGui::Button("LevelUp"))
        sxp_c.xp += sxp_c.xp_for_next_level;
      ImGui::End();
    }
  }

  ui_c.require_level_up = level_up_required;
  if (!level_up_required)
    return;

  // check the probabilities are mathing to 100%
  static_assert(sum_array_values() == 100);

  process_input_for_ui(r, ui_c.state);
  bool& do_act = ui_c.state.do_action;
  int& selected = ui_c.state.selected;

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_NoMove;
  flags |= ImGuiWindowFlags_NoDocking;
  flags |= ImGuiWindowFlags_NoFocusOnAppearing;

  const auto text_col = ImVec4(0.64f, 0.64f, 0.64f, 1.0f);

  const auto& viewport_pos = ImVec2(ri.viewport_pos.x, ri.viewport_pos.y);
  const auto& viewport_size_half = ImVec2(ri.viewport_size_current.x * 0.5f, ri.viewport_size_current.y * 0.5f);
  const auto pos = ImVec2(viewport_pos.x + viewport_size_half.x, viewport_pos.y + viewport_size_half.y);
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize({ 640, 360 }, ImGuiCond_Always);

  int index = 0;
  const float size_x = ImGui::CalcTextSize("Aquire").x;
  const ImVec2 size = { size_x, 13.0f };

  generate_upgrades_for_players(r);

  ImGui::Begin("Level up required!", NULL, flags);
  ImGui::Text("Level-up!");

  const auto view =
    r.view<const PlayerComponent, StatModifierComponent, const TraitComponent, const UpgradeResultsComponent>();
  for (const auto& [e, player_c, stat_c, trait_c, result_c] : view.each()) {

    if (result_c.results.size() == 0)
      continue; // shouldnt occur

    for (const auto& [rarity, upgrade] : result_c.results) {
      const auto rarity_str = std::string(magic_enum::enum_name(rarity));
      const auto upgrade_str = std::string(magic_enum::enum_name(upgrade));

      auto def = SelectableButtonDef{
        .label = "Aquire##" + rarity_str + "_" + upgrade_str,
        .size = size,
        .index = index++,
        .input = do_act,
        .sel_index = selected,
      };

      const auto [amount, type_str] = stat_from_stat_table(rarity, upgrade);

      if (selectable_button(def)) {
        // UpgradeEvent evt;
        // evt.e = e;
        // evt.upgrade = upgrade;
        // evts_c.dispatcher->trigger(evt);
        // evts_c.dispatcher->update();

        // add stats
        if (type_str == "stat_flat_increase")
          stat_c.add(std::make_shared<StatFlatIncrease>(amount, upgrade_str));
        else if (type_str == "stat_percent_increase")
          stat_c.add(std::make_shared<StatPercentIncrease>(amount, upgrade_str));
        else
          throw std::runtime_error("Unknown stat type");

        // remove the upgrades
        r.remove<UpgradeResultsComponent>(e);

        close_ui(sxp_c);
        break;
      }

      // Display rarity
      ImGui::SameLine();
      const auto rarity_to_col = [](Rarity rarity) -> ImVec4 {
        if (rarity == Rarity::COMMON) {
          auto srgb = hex_to_srgb("#b1c9c3"); //  gray
          return { srgb.r / 255.0f, srgb.g / 255.0f, srgb.b / 255.0f, srgb.a / 255.0f };
        }
        if (rarity == Rarity::UNCOMMON) {
          auto srgb = hex_to_srgb("#00c420"); //  green
          return { srgb.r / 255.0f, srgb.g / 255.0f, srgb.b / 255.0f, srgb.a / 255.0f };
        }
        if (rarity == Rarity::RARE) {
          auto srgb = hex_to_srgb("#0096ff"); //  blue
          return { srgb.r / 255.0f, srgb.g / 255.0f, srgb.b / 255.0f, srgb.a / 255.0f };
        }
        if (rarity == Rarity::LEGENDARY) {
          auto srgb = hex_to_srgb("#cfc041"); //  gold
          return { srgb.r / 255.0f, srgb.g / 255.0f, srgb.b / 255.0f, srgb.a / 255.0f };
        }
        if (rarity == Rarity::SUPER_LEGENDARY) {
          auto srgb = hex_to_srgb("#d74200"); //  red
          return { srgb.r / 255.0f, srgb.g / 255.0f, srgb.b / 255.0f, srgb.a / 255.0f };
        }
        return { 1.0f, 1.0f, 1.0f, 1.0f };
      };
      auto col = rarity_to_col(rarity);
      ImGui::TextColored(col, "%s", rarity_str.c_str());

      ImGui::SameLine();

      // Display info about the upgrade
      if (type_str == "stat_flat_increase") {
        auto str = std::format("{} +{:.2f}", upgrade_str, amount);
        ImGui::Text("%s", str.c_str());
      } else {
        auto str = std::format("{} {:.2f}%", upgrade_str, amount);
        ImGui::Text("%s", str.c_str());
      }

      // Description
      // ImGui::SameLine();
      // const std::string desc = generate_description(upgrade);
      // ImGui::TextColored(text_col, "%s", desc.c_str());
    }
  }

  ui_c.state.max = index;
  ImGui::End();
}

} // namespace game2d