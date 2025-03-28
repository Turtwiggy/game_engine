#include "pch.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/sprites/helpers.hpp"
#include "modules/controller_input_update_ui/controller_input_update_ui_helpers.hpp"
#include "modules/core_io/io_helpers.hpp"
#include "modules/core_renderer/components.hpp"
#include "modules/system_item_gold/gold_components.hpp"
#include "modules/system_upgrade/upgrade_components.hpp"
#include "modules/ui_colours/ui_colours_helpers.hpp"
#include "modules/ui_common/ui_common_helpers.hpp"
#include "modules/ui_scene_main_menu/ui_scene_main_menu_components.hpp"
#include "ui_scene_upgrades_components.hpp"
#include "ui_scene_upgrades_system.hpp"

namespace game2d {
using namespace std::literals;

const int columns = 4;

void
purchase_upgrade(entt::registry& r, const UpgradeableStat stat)
{
  GET_FIRST_OR_RETURN(SINGLE_Upgrades, r, upgrade_e, upgrade_c);
  GET_FIRST_OR_RETURN(SINGLE_GoldComponent, r, gold_e, gold_c);

  const auto stat_str = std::string(magic_enum::enum_name<UpgradeableStat>(stat));

  const auto find_by_key = [&stat_str](Upgrade& u) { return u.key == stat_str; };
  const auto it = std::find_if(upgrade_c.upgrades.begin(), upgrade_c.upgrades.end(), find_by_key);
  if (it == upgrade_c.upgrades.end()) {
    auto err = std::format("Upgrade does not exist: {}", stat_str);
    throw std::runtime_error(err.c_str());
  }
  const Upgrade u = (*it);
  SDL_Log("You want to purchase: %s. It has %zu levels available", u.key.c_str(), u.levels.size());

  // your current level.
  std::optional<int> your_level = std::nullopt;

  const auto ondisk_opt = savefile_get_key(r, stat_str);
  if (ondisk_opt.has_value()) {
    const auto ondisk_json = ondisk_opt.value();
    ondisk_json.get_to<int>(your_level.emplace());
    SDL_Log("Your current stat level is: %i", your_level.value());
  }

  std::optional<UpgradeLevel> next_ul = std::nullopt;

  // you haven't aquired any levels.
  if (your_level == std::nullopt) {
    if (u.levels.size() > 0)
      next_ul = u.levels[0]; // the first upgrade
  }

  // you have aquired some levels.
  if (your_level != std::nullopt && your_level.value() < u.levels.size())
    next_ul = u.levels[your_level.value()];

  // check: you've already bought the skill-tree to max
  if (!next_ul.has_value()) {
    SDL_Log("Skill-tree at max");
    return;
  }

  // check: you have enough gold.
  if (next_ul.value().cost > gold_c.amount) {
    SDL_Log("Need more gold.");
    return;
  }

  if (your_level.has_value())
    savefile_put_key(r, stat_str, your_level.value() + 1);
  else
    savefile_put_key(r, stat_str, 1);

  // Buy the skill.
  gold_c.amount -= (int)(next_ul.value().cost);

  savefile_put_key(r, "GOLD_AMOUNT", gold_c.amount);
  savefile_save_disk(r);
};

void
init_menu(entt::registry& r, SINGLE_UpgradesMenuUI& ui_c)
{
  ui_c.state.resize(columns);

  for (int i = 0; i < (int)UpgradeableStat::count; i++) {

    const auto stat_enum = magic_enum::enum_value<UpgradeableStat>(i);
    const auto stat_str = std::string(magic_enum::enum_name<UpgradeableStat>(stat_enum));

    // stats to skip...
    if (stat_str.find("ACTOR_STAMINA") != std::string::npos)
      continue;

    // Hack: move rows to columns based on name...
    int ui_col = 0;
    if (stat_str.find("ACTOR_") != std::string::npos)
      ui_col = 0;
    if (stat_str.find("BULLET_") != std::string::npos)
      ui_col = 1;
    if (stat_str.find("WEAPON_") != std::string::npos)
      ui_col = 2;

    ui_c.state[ui_col].rows.push_back(
      RowState{ .col_name = stat_str.c_str(), .action = [&ui_c, stat_enum]() { ui_c.selected_stat = stat_enum; } });
  }

  // a third column for aquire/back to menu?
  const int ui_col = 3;

  ui_c.state[ui_col].rows.push_back(RowState{ .col_name = "Aquire", .action = [&r, &ui_c]() {
                                               if (ui_c.selected_stat.has_value())
                                                 purchase_upgrade(r, ui_c.selected_stat.value());
                                             } });

  ui_c.state[ui_col].rows.push_back(RowState{ .col_name = "Back To Menu", .action = [&r, &ui_c]() {
                                               ui_c.display = false;
                                               ui_c.one_frame_buffer = true;
                                               ui_c.selected_stat = std::nullopt;

                                               ui_c.ui_col_index = 0;

                                               for (int i = 0; i < ui_c.state.size(); i++) {
                                                 auto& state = ui_c.state[i];
                                                 state.current_row_index = 0;
                                               }

                                               create_empty<RequestToShowMainMenu>(r);
                                             } });
};

void
ui_gold(entt::registry& r)
{
  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);

  // show a sound icon
  ImGuiWindowFlags icon_flags = 0;
  icon_flags |= ImGuiWindowFlags_NoCollapse;
  icon_flags |= ImGuiWindowFlags_NoTitleBar;
  icon_flags |= ImGuiWindowFlags_AlwaysAutoResize;
  // icon_flags |= ImGuiWindowFlags_NoBackground;

  const ImVec2 icon_size{ 50, 50 };
  const auto [tl, br] = convert_sprite_to_uv(r, "COINPILE_1"s);

  const float distance_from_left_of_screen = 0;
  const float distance_from_top_of_screen = 75;

  const ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(
    ImVec2(viewport->WorkPos.x + distance_from_left_of_screen, viewport->WorkPos.y + distance_from_top_of_screen));

  ImGui::Begin("Gold", nullptr, icon_flags);

  auto& gold_c = get_first_component<SINGLE_GoldComponent>(r);
  ImGui::Text("GOLD: %i", gold_c.amount);

  ImGui::End();
}

void
update_ui_scene_upgrades_system(entt::registry& r)
{
  GET_FIRST_OR_RETURN(SINGLE_UpgradesMenuUI, r, ui_e, ui_c)
  GET_FIRST_OR_RETURN(SINGLE_RendererInfo, r, ri_e, ri)
  GET_FIRST_OR_RETURN(SINGLE_Upgrades, r, upgrade_e, upgrade_c);
  auto& gold_c = get_first_component<SINGLE_GoldComponent>(r);

  if (!ui_c.init) {
    init_menu(r, ui_c);
    ui_c.init = true;
  }

  process_requests<RequestToShowUpgradesMenu>(r, [&ui_c]() { ui_c.display = true; });

  if (!ui_c.display)
    return;

  // prevent immediately doing actions
  if (ui_c.one_frame_buffer) {
    SDL_Log("Upgrade menu... one frame buffer...");
    ui_c.one_frame_buffer = false;
    return;
  }

  int active_column_idx = ui_c.ui_col_index;
  process_input_for_ui_all_handles(r, ui_c.state[active_column_idx]);

  // check if left or right was pressed.
  {
    auto& active_col = ui_c.state[active_column_idx];
    for (auto& row : active_col.rows) {
      if (row.col_index == 0)
        continue;

      // process moving column.
      if (row.col_index > 0)
        ui_c.ui_col_index++;
      if (row.col_index < 0)
        ui_c.ui_col_index--;

      // clamp the col index
      const int max = columns;
      ui_c.ui_col_index = ui_c.ui_col_index < 0 ? max - 1 : ui_c.ui_col_index;
      ui_c.ui_col_index %= max;

      // process moving row
      auto& new_active_row = ui_c.state[ui_c.ui_col_index];
      new_active_row.current_row_index = active_col.current_row_index;

      // clamp the row index
      const int row_max = (int)new_active_row.rows.size();
      int& cri = new_active_row.current_row_index;
      cri = cri < 0 ? row_max - 1 : cri;
      cri %= row_max;

      // set it back to 0
      row.col_index = 0;
    }
  }
  auto& active_column = ui_c.state[active_column_idx];

#if defined(_DEBUG)
  // static bool debug_menu = true;
  // if (debug_menu) {
  //   ImGui::Text("Rows: %zu", ui_c.state.rows.size());
  //   for (const auto& row : ui_c.state.rows)
  //     ImGui::Text("%s, idx: %i", row.col_name.c_str(), row.col_index);
  // }

  if (ImGui::Button("GiveGold"))
    gold_c.amount += 5;

#endif

  ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, { 0.0f, 0.5f });
  // ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 0.0f, 0.0f });
  // ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
  // ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 10.0f, 10.0f });
  // ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoCollapse;
  flags |= ImGuiWindowFlags_NoTitleBar;
  flags |= ImGuiWindowFlags_AlwaysAutoResize;
  // flags |= ImGuiWindowFlags_NoBackground;

  const auto viewport_pos = ImVec2((float)ri.viewport_pos.x, (float)ri.viewport_pos.y);
  const auto viewport_size = ImVec2(ri.viewport_size_render_at.x, ri.viewport_size_render_at.y);
  const auto viewport_size_half = ImVec2(ri.viewport_size_render_at.x * 0.5f, ri.viewport_size_render_at.y * 0.5f);

  const auto pos = ImVec2(viewport_pos.x + viewport_size_half.x, viewport_pos.y + viewport_size_half.y);
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.0f));

  ImGui::Begin("Upgrades Menu", NULL, flags);

  ImGui::Text("Gold: %i", gold_c.amount);

  const auto TEXT_SIZE = ImGui::CalcTextSize("A");
  const ImVec2 button_size = { 200.0f, TEXT_SIZE.y + 2.0f };

  const bool do_act = std::find(active_column.new_actions.begin(), active_column.new_actions.end(), UIAction::SELECT) !=
                      active_column.new_actions.end();

  // int& selected = active_column.current_row_index;

  if (ImGui::BeginTable("upgrades", columns)) {

    //
    // 4 columns, of varying size of rows.
    //

    int max_rows = 0;
    for (int col_idx = 0; col_idx < columns; col_idx++) {
      const auto& rows = ui_c.state[col_idx].rows;
      max_rows = std::max(max_rows, (int)rows.size());
    }

    for (int row_idx = 0; row_idx < max_rows; row_idx++) {
      ImGui::TableNextRow();

      for (int col_idx = 0; col_idx < columns; col_idx++) {
        ImGui::TableSetColumnIndex(col_idx);

        const auto& rows = ui_c.state[col_idx].rows;
        if (row_idx < rows.size()) {
          // data
          const auto row = rows[row_idx];
          const bool ui_col_active = ui_c.ui_col_index == col_idx;

          const auto stat_key = row.col_name;

          int n_stat_upgrades_aquired = 0;
          const auto on_disk_stat_level_opt = savefile_get_key(r, stat_key);
          if (on_disk_stat_level_opt.has_value()) {
            const nlohmann::json data = on_disk_stat_level_opt.value();
            data.get_to(n_stat_upgrades_aquired);
          }

          int n_stat_upgrades = 0;
          const auto find_by_key = [&stat_key](Upgrade& u) { return u.key == stat_key; };
          const auto it = std::find_if(upgrade_c.upgrades.begin(), upgrade_c.upgrades.end(), find_by_key);
          if (it != upgrade_c.upgrades.end())
            n_stat_upgrades = (*it).levels.size();

          // remove ACTOR_ from the display key
          auto display_stat_key = row.col_name;
          const std::string str_to_remove = "ACTOR_";
          const auto actor_pos = display_stat_key.find(str_to_remove);
          if (actor_pos != std::string::npos)
            display_stat_key = display_stat_key.substr(str_to_remove.length(), display_stat_key.length());

          // Prefix the button with the your_aquired out of available_aquired
          std::string button_str = "";
          if (n_stat_upgrades > 0)
            button_str = std::format("{}/{} {}", n_stat_upgrades_aquired, n_stat_upgrades, display_stat_key);
          else
            button_str = std::format("{}", display_stat_key);

          auto a_def = SelectableButtonDef{
            .label = button_str,
            .size = button_size,
            .input = ui_col_active && do_act,
            .my_row_index = row_idx,
            .my_col_index = col_idx,
            .ui_row_index = ui_c.state[col_idx].current_row_index,
            .ui_col_index = ui_c.ui_col_index,
            .ui_col_active = ui_col_active,
          };

          if (selectable_button(r, a_def))
            row.action();
        } else {
          // no data
        }
      }
    }

    ImGui::EndTable();
  }

  // Perma upgradeable stats...

  ImGui::Text("Selected Stat...");

  if (ui_c.selected_stat.has_value()) {
    const auto stat_str = std::string(magic_enum::enum_name<UpgradeableStat>(ui_c.selected_stat.value()));

    const auto find_by_key = [&stat_str](Upgrade& u) { return u.key == stat_str; };
    const auto it = std::find_if(upgrade_c.upgrades.begin(), upgrade_c.upgrades.end(), find_by_key);
    if (it == upgrade_c.upgrades.end()) {
      const auto err = std::format("Upgrade does not exist: {}", stat_str);
      throw std::runtime_error(err.c_str());
    }
    const Upgrade u = (*it);
    ImGui::Text("%s. Levels available: %zu", u.key.c_str(), u.levels.size());

    // loaded on-disk values
    int your_level = 0;
    auto str_opt = savefile_get_key(r, stat_str);
    if (str_opt.has_value())
      str_opt->get_to(your_level);

    // TODO: display the current upgrade level, and future upgrade levels
    for (int i = 0; i < u.levels.size(); i++) {
      const UpgradeLevel& l = u.levels[i];
      const bool aquired = i < your_level;

      static auto aquired_col = convert_my_to_im_vec(engine::SRGBColour(0, 255, 0, 255));
      static auto unaquired_col = convert_my_to_im_vec(engine::SRGBColour(255, 255, 255, 255));

      std::string str = "";
      if (l.type == "stat_percent_increase")
        str = std::format("Cost: {}G. Increase +{}%", l.cost, l.value, aquired);
      else if (l.type == "stat_flat_increase")
        str = std::format("Cost: {}G. Increase +{}", l.cost, l.value, aquired);

      if (aquired)
        ImGui::TextColored(aquired_col, "%s", str.c_str());
      else
        ImGui::TextColored(unaquired_col, "%s", str.c_str());
    }

  } else
    ImGui::Text("None...");

  ImGui::End();
  ImGui::PopStyleVar(1);

  // ui_gold(r); // display gold in top left
}

} // namespace game2d