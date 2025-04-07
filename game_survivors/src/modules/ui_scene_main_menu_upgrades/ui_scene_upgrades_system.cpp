#include "pch.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/imgui/helpers.hpp"
#include "modules/actor_weapon/weapon_components.hpp"
#include "modules/controller_input_update_ui/controller_input_update_ui_helpers.hpp"
#include "modules/core_io/io_helpers.hpp"
#include "modules/core_renderer/components.hpp"
#include "modules/system_hardpoint_arcs/hulls_components.hpp"
#include "modules/system_item_gold/gold_components.hpp"
#include "modules/system_upgrade/upgrade_components.hpp"
#include "modules/ui_colours/ui_colours_helpers.hpp"
#include "modules/ui_common/ui_common_components.hpp"
#include "modules/ui_common/ui_common_helpers.hpp"
#include "modules/ui_scene_main_menu/ui_scene_main_menu_components.hpp"
#include "ui_scene_upgrades_components.hpp"
#include "ui_scene_upgrades_system.hpp"

namespace game2d {
using namespace std::literals;

const int data_columns = 5; // one hidden col
const int table_columns = 4;
const int hidden_col_idx = 4;
const auto my_window_bg_col = hex_to_srgb("#21242B");
const auto im_window_bg_col = convert_my_to_im(my_window_bg_col);
const auto my_window_border_col = hex_to_srgb("#3B5676");
const auto im_window_border_col = convert_my_to_im(my_window_bg_col);
const auto rounding = 4.0f;
const auto thickness = 1.0f;
const auto rect_flags = ImDrawFlags_RoundCornersAll;

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
ui_display_backbuttons(entt::registry& r, SINGLE_UpgradesMenuUI& ui_c)
{
  const auto font_scale = get_first_component<SINGLE_UIData>(r).scaling;
  const auto text_font_enum = font_scale == 1.0f ? FontSize::TEXT_SIZE_13 : FontSize::TEXT_SIZE_13_SCALED;
  const auto text_font_size = (float)text_font_enum;
  auto* text_font = get_inter_font(r, text_font_enum);
  const auto TEXT_SIZE = text_font->CalcTextSizeA(text_font_size, FLT_MAX, -1, "A");
  const auto button_size = ImVec2{ 100.0f, TEXT_SIZE.y + 2.0f };

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_AlwaysAutoResize;
  flags |= ImGuiWindowFlags_NoBackground;

  // const ImGuiViewport* viewport = ImGui::GetMainViewport();
  // ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, viewport->WorkPos.y));
  // auto& back_buttons_c = get_first_component<SINGLE_UIBackButtons>(r);

  ImGui::PushFont(text_font);
  ImGui::Begin("BackButtons", NULL, flags);

  auto& col = ui_c.state[hidden_col_idx];
  const bool do_act = std::find(col.actions.begin(), col.actions.end(), UIAction::SELECT) != col.actions.end();

  for (int i = 0; i < col.rows.size(); i++) {
    auto& row = col.rows[i];
    const bool ui_col_active = ui_c.ui_col_index == hidden_col_idx;

    auto a_def = SelectableButtonDef{
      .label = row.col_name,
      .size = button_size,
      .input = ui_col_active && do_act,

      .my_row_index = i,
      .my_col_index = hidden_col_idx,
      .ui_row_index = ui_c.state[hidden_col_idx].current_row_index,
      .ui_col_index = ui_c.ui_col_index,
      .ui_col_active = ui_col_active,

      .font = text_font,

      // hide the buttons
      // .active_outline_col = { 0.0f, 0.0f, 0.0f, 0.0f },
      // .inactive_outline_col = { 0.0f, 0.0f, 0.0f, 0.0f },
      // .active_bg_col = { 0.0f, 0.0f, 0.0f, 0.0f },
      // .inactive_bg_col = { 0.0f, 0.0f, 0.0f, 0.0f },
    };
    if (selectable_button(r, a_def))
      row.action();
  }

  ImGui::End();
  ImGui::PopFont();
};

void
init_menu(entt::registry& r, SINGLE_UpgradesMenuUI& ui_c)
{
  ui_c.state.resize(data_columns);

  // col_idx: 0: hulls
  auto& hulls_c = get_first_component<SINGLE_Hulls>(r);
  for (int i = 0; i < hulls_c.hulls.size(); i++) {
    const auto& hull = hulls_c.hulls[i];
    ui_c.state[0].rows.push_back(RowState{ .col_name = hull.name, .action = []() {} });
  }

  // col_idx 1: weapons
  auto& weapons_c = get_first_component<SINGLE_Weapons>(r);
  for (int i = 0; i < weapons_c.weapons.size(); i++) {
    const auto& weapon = weapons_c.weapons[i];
    ui_c.state[1].rows.push_back(RowState{ .col_name = weapon.name, .action = []() {} });
  }

  // col_idx 2: abilities
  ui_c.state[2].rows.push_back((RowState{ .col_name = "None", .action = []() {} }));

  // col_idx 3: stats.
  for (int i = 0; i < (int)UpgradeableStat::count; i++) {

    const auto stat_enum = magic_enum::enum_value<UpgradeableStat>(i);
    const auto stat_str = std::string(magic_enum::enum_name<UpgradeableStat>(stat_enum));

    // stats to skip...
    if (stat_str.find("ACTOR_STAMINA") != std::string::npos)
      continue;

    // Hack: Move stats to the stats column
    int ui_col = 0;
    if (stat_str.find("ACTOR_") != std::string::npos)
      ui_col = 3;
    if (stat_str.find("BULLET_") != std::string::npos)
      ui_col = 3;
    if (stat_str.find("WEAPON_") != std::string::npos)
      ui_col = 3;

    ui_c.state[ui_col].rows.push_back(RowState{ .col_name = stat_str.c_str(), .action = [&ui_c, stat_enum]() {
                                                 ui_c.selected_stat = stat_enum;
                                                 ui_c.ui_col_index = hidden_col_idx; // go to hidden col
                                               } });
  }

  // hidden col 4: aquire, back to select, back to menu

  ui_c.state[hidden_col_idx].rows.push_back(RowState{ .col_name = "(Clear)", .action = [&ui_c]() {
                                                       // clear the selected stat
                                                       ui_c.selected_stat = std::nullopt;
                                                       ui_c.ui_col_index = 0;
                                                       ui_c.state[hidden_col_idx].current_row_index = 0;
                                                     } });

  ui_c.state[hidden_col_idx].rows.push_back(RowState{ .col_name = "Buy", .action = [&r, &ui_c]() {
                                                       // purchase the upgrade
                                                       if (ui_c.selected_stat.has_value())
                                                         purchase_upgrade(r, ui_c.selected_stat.value());
                                                     } });

  ui_c.state[hidden_col_idx].rows.push_back(RowState{ .col_name = "Menu", .action = [&r, &ui_c]() {
                                                       // Back to menu
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

  process_requests<RequestToShowUpgradesMenu>(r, [&ui_c](const auto& req) { ui_c.display = true; });

  if (!ui_c.display)
    return;

  // prevent immediately doing actions
  if (ui_c.one_frame_buffer) {
    SDL_Log("Upgrade menu... one frame buffer...");
    ui_c.one_frame_buffer = false;
    return;
  }

  // fonts
  const auto font_scale = get_first_component<SINGLE_UIData>(r).scaling;

  // idx: 3 should be fingerpaint, idx: 4 should be fingerpaint scaled.
  auto* fingerpaint_font = ImGui::GetIO().Fonts->Fonts[font_scale == 1.0f ? 3 : 4];

  const auto header_font_enum = font_scale == 1.0f ? FontSize::TEXT_SIZE_16 : FontSize::TEXT_SIZE_16_SCALED;
  const auto header_font_size = (float)header_font_enum;
  auto* header_font = get_inter_font(r, header_font_enum);
  const auto text_font_enum = font_scale == 1.0f ? FontSize::TEXT_SIZE_13 : FontSize::TEXT_SIZE_13_SCALED;
  const auto text_font_size = (float)text_font_enum;
  auto* text_font = get_inter_font(r, text_font_enum);
  const auto TEXT_SIZE = text_font->CalcTextSizeA(text_font_size, FLT_MAX, -1, "A");

  // clear all actions.
  for (int i = 0; i < data_columns; i++)
    ui_c.state[i].actions.clear();

  // process actions.
  int active_column_idx = ui_c.ui_col_index;
  process_input_for_ui_all_handles(r, ui_c.state[active_column_idx]);

  // check if left or right was pressed.
  {
    // Only process inputs if not on hidden column idx: 4
    if (active_column_idx != hidden_col_idx) {

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
        const int max = table_columns;
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

  static glm::vec2 a{ 0.0f, 0.0f };
  static glm::vec2 b{ 10.0f, 12.0f };
  static glm::vec2 c{ 0.0f, 4.0f };
#if defined(_DEBUG)
  imgui_draw_vec2("a", a);
  imgui_draw_vec2("b", b);
  imgui_draw_vec2("c", c);
#endif

  ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, { 0.0f, 0.5f });
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ a.x, a.y });
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ b.x, b.y });
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ c.x, c.y });
  ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2{ 0, 1 });

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoCollapse;
  flags |= ImGuiWindowFlags_NoTitleBar;
  flags |= ImGuiWindowFlags_AlwaysAutoResize;
  flags |= ImGuiWindowFlags_NoSavedSettings;
  // flags |= ImGuiWindowFlags_NoBackground;

  const auto viewport_pos = ImVec2((float)ri.viewport_pos.x, (float)ri.viewport_pos.y);
  const auto viewport_size = ImVec2((float)ri.viewport_size_render_at.x, (float)ri.viewport_size_render_at.y);
  const auto viewport_size_half = ImVec2(ri.viewport_size_render_at.x * 0.5f, ri.viewport_size_render_at.y * 0.5f);

  const float upper = (viewport_pos.y + 0.15f * viewport_size.y);
  const float lower = (viewport_pos.y + 0.75f * viewport_size.y);

  const auto pos = ImVec2(viewport_pos.x + viewport_size_half.x, upper);
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.0f));

  // const float max_size_y = lower - upper;
  // ImGui::SetNextWindowSizeConstraints({ 0, 0 }, { (float)ri.viewport_size_render_at.x, (float)max_size_y });

  ImGui::Begin("Upgrades Menu", NULL, flags);
  const auto ui_tl = ImGui::GetWindowPos();
  const auto ui_wh = ImGui::GetWindowSize();
  const auto ui_br = ImVec2{ ui_tl.x + ui_wh.x, ui_tl.y + ui_wh.y };

  // background
  {
    const auto edges_tl = ImVec2{ ui_tl.x + thickness, ui_tl.y + thickness };
    const auto edges_br = ImVec2{ ui_br.x - thickness, ui_br.y - thickness };

    auto* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRectFilled(ui_tl, ui_br, im_window_bg_col, rounding);
    draw_list->AddRect(edges_tl, edges_br, IM_COL32(255, 255, 255, 255), rounding, rect_flags, thickness);
  }

  ImGui::PushFont(fingerpaint_font);

  ImGui::TextColored(ImVec4(255 * 1.0f, 238 * 1.0f, 0 * 1.0f, 1.0f), "G: %i", gold_c.amount);

  auto upgr_text = "Upgrades";
  auto upgr_size = fingerpaint_font->CalcTextSizeA(fingerpaint_font->FontSize, FLT_MAX, -1, upgr_text);
  ImGui::SameLine();
  ImGui::SetCursorScreenPos({ ui_tl.x + (ui_wh.x * 0.5f) - (upgr_size.x * 0.5f), ui_tl.y });
  ImGui::Text("%s", upgr_text);

  ImGui::PopFont();

  const ImVec2 button_size = { 200.0f, TEXT_SIZE.y + 2.0f };

  auto& acts = active_column.actions;
  const bool do_act = std::find(acts.begin(), acts.end(), UIAction::SELECT) != acts.end();
  // if (active_column_idx != hidden_col_idx)
  //   acts.clear(); // consume

  // int& selected = active_column.current_row_index;

  ImGui::PushFont(text_font);

  // Calculate max rows.
  int max_rows = 0;
  for (int col_idx = 0; col_idx < table_columns; col_idx++) {
    const auto& rows = ui_c.state[col_idx].rows;
    max_rows = std::max(max_rows, (int)rows.size());
  }

  const ImGuiTableFlags table_flags = ImGuiTableFlags_ScrollY;
  // const int cols_to_display = max_rows;
  // const float y_size = TEXT_SIZE.y * cols_to_display;
  // if (ImGui::BeginTable("upgrades", columns, table_flags, ImVec2(0.0f, y_size))) {
  if (ImGui::BeginTable("upgrades", table_columns, table_flags)) {

    //
    // 4 columns, of varying size of rows.
    //

    for (int row_idx = 0; row_idx < max_rows; row_idx++) {
      ImGui::TableNextRow();

      for (int col_idx = 0; col_idx < table_columns; col_idx++) {
        ImGui::TableSetColumnIndex(col_idx);

        const auto& rows = ui_c.state[col_idx].rows;
        if (row_idx >= rows.size())
          continue; // no data.

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

          // only let the mouse update the stat selection
          // if no stat is currently selected
          .update_selected_on_mouse_move = !ui_c.selected_stat.has_value(),

          .font = text_font,

          // hide the buttons
          .active_outline_col = { 0.0f, 0.0f, 0.0f, 0.0f },
          .inactive_outline_col = { 0.0f, 0.0f, 0.0f, 0.0f },
          .active_bg_col = { 0.0f, 0.0f, 0.0f, 0.0f },
          .inactive_bg_col = { 0.0f, 0.0f, 0.0f, 0.0f },
        };

        if (selectable_button(r, a_def))
          row.action();

        // Set the display stat as the actively selected stat
        // if (ui_col_active) {
        //   const int i = row_idx;
        //   const auto stat_enum = magic_enum::enum_value<UpgradeableStat>(i);
        //   const auto stat_str = std::string(magic_enum::enum_name<UpgradeableStat>(stat_enum));
        //   ui_c.selected_stat = stat_enum;
        // }

        //
      }
    }

    ImGui::EndTable();
  }

  ImGui::PopFont();
  ImGui::End();

  ImGui::SetNextWindowPos({ ri.viewport_size_render_at.x * 0.5f, lower }, ImGuiCond_Always, { 0.5f, 0.0f });

  ImGui::Begin("SelectedStat", NULL, flags);
  const auto stat_tl = ImGui::GetWindowPos();
  const auto stat_wh = ImGui::GetWindowSize();
  const auto stat_br = ImVec2{ stat_tl.x + stat_wh.x, stat_tl.y + stat_wh.y };

  // background
  {
    auto* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRectFilled(stat_tl, stat_br, im_window_bg_col, rounding);
    draw_list->AddRect(stat_tl, stat_br, IM_COL32(255, 255, 255, 255), rounding, rect_flags, thickness);
  }

  // Perma upgradeable stats...

  if (ui_c.selected_stat.has_value()) {
    const auto stat_str = std::string(magic_enum::enum_name<UpgradeableStat>(ui_c.selected_stat.value()));

    const auto find_by_key = [&stat_str](Upgrade& u) { return u.key == stat_str; };
    const auto it = std::find_if(upgrade_c.upgrades.begin(), upgrade_c.upgrades.end(), find_by_key);
    if (it == upgrade_c.upgrades.end()) {
      const auto err = std::format("Upgrade does not exist: {}", stat_str);
      throw std::runtime_error(err.c_str());
    }
    const Upgrade u = (*it);
    // ImGui::Text("%s. Available: %zu", u.key.c_str(), u.levels.size());
    ImGui::Text("%s", u.key.c_str());

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
        str = std::format("{}G. +{}%", l.cost, l.value, aquired);
      else if (l.type == "stat_flat_increase")
        str = std::format("{}G. +{}", l.cost, l.value, aquired);

      if (aquired)
        ImGui::TextColored(aquired_col, "%s", str.c_str());
      else
        ImGui::TextColored(unaquired_col, "%s", str.c_str());
    }

  } else
    ImGui::Text("Select upgrade...");

  ImGui::End();

  ImGui::SetNextWindowPos({ stat_tl.x + stat_wh.x, stat_tl.y }, ImGuiCond_Always, { 0, 0 });
  ui_display_backbuttons(r, ui_c);

  ImGui::PopStyleVar(7);
}

} // namespace game2d