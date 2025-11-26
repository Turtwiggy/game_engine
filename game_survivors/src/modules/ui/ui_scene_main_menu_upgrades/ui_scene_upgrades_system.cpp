#include "pch.hpp"

#include "engine/audio/audio_components.hpp"
#include "engine/deps/opengl.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/imgui/ui_imgui_defaults.hpp"
#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"
#include "engine/opengl/texture.hpp"
#include "engine/sprites/helpers.hpp"
#include "engine/std/string/helpers.hpp"
#include "entt/entity/fwd.hpp"
#include "modules/actors/actor_weapon/weapon_components.hpp"
#include "modules/core/fonts/fonts_helpers.hpp"
#include "modules/core/io/io_helpers.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/renderer/helpers.hpp"
#include "modules/core/ui/ui_common_components.hpp"
#include "modules/core/ui/ui_common_helpers.hpp"
#include "modules/systems/system_hardpoint_arcs/hulls_components.hpp"
#include "modules/systems/system_item_gold/gold_components.hpp"
#include "modules/systems/system_persistent_upgrades/persistent_upgrade_components.hpp"
#include "modules/systems/system_shop/shop_helpers.hpp"
#include "modules/systems/system_upgrade/upgrade_components.hpp"
#include "modules/ui/ui_colours/ui_colours_helpers.hpp"
#include "modules/ui/ui_element_cursor/element_cursor_components.hpp"
#include "modules/ui/ui_element_cursor/element_cursor_helpers.hpp"
#include "modules/ui/ui_scene_survive_upgrade/ui_survive_upgrade_helpers.hpp"
#include "resources/data.hpp"
#include "ui_scene_upgrades_components.hpp"
#include "ui_scene_upgrades_helpers.hpp"
#include "ui_scene_upgrades_system.hpp"

namespace game2d {
using namespace std::literals;

void
draw_moneybag(entt::registry& r, ImVec2 ui_tl, ImVec2 ui_wh, ImFont* header_font, float header_font_size, int gold)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  GET_FIRST_OR_RETURN(SINGLE_PersistentUpgradesMenuUI, r, ui_e, ui_c)
  GET_FIRST_OR_RETURN(SINGLE_PersistentUpgrades, r, upgrade_e, upgrade_c);
  auto& ri_c = SINGLE_RendererInfo::instance;
  const auto font_scale = get_first_component<SINGLE_UIScaling>(r).scaling;
  const auto monochrome_tex_id = search_for_texture_id_by_texture_path(ri_c, "monochrome")->id;
  const auto monochrome_im_id = (ImTextureID)(void*)(intptr_t)monochrome_tex_id;
  auto* draw_list = ImGui::GetWindowDrawList();
  const auto ui_br = ui_tl + ui_wh;

  // draw the amount of gold you have.
  const auto gold_txt = std::format(" {}", gold);
  ImGui::PushFont(header_font, header_font_size);
  const auto gold_wh = ImGui::CalcTextSize(gold_txt.c_str());
  ImGui::PopFont();

  const auto icon_size = ImVec2{ 16, 16 };
  const auto inset = ImVec2{ -6, 0 };
  const auto icon_y = ui_tl.y + (ui_wh.y - icon_size.y) * 0.5f;

  const auto icon_tl = ImVec2{ ui_br.x - icon_size.x - gold_wh.x + inset.x, icon_y };
  const auto icon_br = ImVec2{ icon_tl.x + icon_size.x, icon_tl.y + icon_size.y };

  // draw a moneybag for your gold
  const auto [uv_tl, uv_br] = convert_sprite_to_uv(r, "COINPILE_1"s);
  draw_list->AddImage(monochrome_im_id, icon_tl, icon_br, uv_tl, uv_br, im_gold_col);

  draw_list->AddText(header_font, header_font_size, ImVec2{ icon_br.x, icon_tl.y }, im_text_col, gold_txt.c_str());
}

void
draw_header_row(entt::registry& r, SINGLE_PersistentUpgradesMenuUI& ui_c, ImVec2 button_tl, bool do_act, float dt)
{
  auto* inter_font = get_inter_font(r);
  auto* text_font = inter_font;
  const auto font_text_size = (float)FontSizes::SIZE_16;

  const float button_inbetween_space_x = 10.0f;
  const glm::vec2 button_padding{ 20, 13.5 };
  // #if defined(_DEBUG)
  // ImGui::Begin("Debug");
  // imgui_draw_vec2("button_padding", button_padding);
  // ImGui::End();
  // #endif
  ImVec2 button_header_pos = { button_tl.x + button_padding.x, button_tl.y + button_padding.y };

  const float offset = -14;
  // ImGui::Begin("Debug");
  // imgui_draw_float("offset", offset);
  // ImGui::End();

  for (int i = 0; i < (int)ui_c.state.cells.size(); i++) {
    std::shared_ptr<Cell> cell = ui_c.state.cells[i];
    HeaderCell* base = dynamic_cast<HeaderCell*>(cell.get());
    if (!base)
      continue;

    ImGui::PushFont(text_font, font_text_size);
    ImVec2 text_wh = ImGui::CalcTextSize(base->name.c_str());
    ImGui::PopFont();

    ImVec2 button_wh = text_wh;
    button_wh.y = 16;

    SelectableButtonDef header_button_def = {
      .display_str = base->name,
      .imgui_hash = "##" + base->name,
      .size = button_wh,
      .input = do_act,
      .cell = cell,
      .active_cell = ui_c.active_header,
      .update_selected_on_mouse_move = false,

      .font = text_font,
      .font_size = font_text_size,

      // hide the buttons
      .active_outline_col = { 0.0f, 0.0f, 0.0f, 0.0f },
      .inactive_outline_col = { 0.0f, 0.0f, 0.0f, 0.0f },
      .active_bg_col = { 0.3f, 0.3f, 0.3f, 0.0f },
      .inactive_bg_col = { 0.0f, 0.0f, 0.0f, 0.0f },
    };

    // if the header is clicked, make it the active header
    ImGui::SetCursorScreenPos(button_header_pos);
    if (selectable_button(r, header_button_def))
      ui_c.active_header = cell;

    // additionally, if the cell is active, make it the active header
    bool active = ui_c.state.active == cell;
    if (active)
      ui_c.active_header = cell;

    // draw a cursor on the active cell
    auto pos_tl = button_header_pos;
    pos_tl.x += 0.5f * text_wh.x;
    pos_tl.x += offset;

    if (active)
      draw_cursor(r, ui_c.cursor_c, pos_tl, dt);

    // move horizontally
    button_header_pos.x += button_wh.x + button_inbetween_space_x;
  }
};

void
update_ui_scene_upgrades_system(entt::registry& r, const float dt)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  GET_FIRST_OR_RETURN(SINGLE_PersistentUpgradesMenuUI, r, ui_e, ui_c)
  GET_FIRST_OR_RETURN(SINGLE_PersistentUpgrades, r, upgrade_e, upgrade_c);
  auto& ri_c = SINGLE_RendererInfo::instance;
  auto& gold_c = get_first_component<SINGLE_GoldComponent>(r);
  const auto& shop_c = get_first_component<SINGLE_Shop>(r);

  if (!ui_c.init)
    ui_c.do_init(r);

  ui_c.update<RequestToShowUpgradesMenu>(r);
  if (!ui_c.open)
    return;

  // process actions.
  process_input_for_ui_all_handles(r, ui_c.state);
  // process_input_for_grid(r, ui_c);

  const auto g_input_e = get_first<InputComponent, Persistent>(r);
  const auto& g_input_c = r.get<InputComponent>(g_input_e);
  const auto& b_s = g_input_c.button_s;
  const auto& b_e = g_input_c.button_e;
  bool hel_sel = std::find(b_s.begin(), b_s.end(), ActionStateEnum::HELD) != b_s.end();
  const bool rel_sel = std::find(b_s.begin(), b_s.end(), ActionStateEnum::RELEASE) != b_s.end();
  const bool do_back = std::find(b_e.begin(), b_e.end(), ActionStateEnum::DOWN) != b_e.end();
  const bool do_act = std::find(b_s.begin(), b_s.end(), ActionStateEnum::DOWN) != b_s.end();

  // also held if mouse lmb clicked.
  hel_sel |= ImGui::IsMouseDown(ImGuiMouseButton_Left);

  bool do_purchase = false;

  // hold to purchase upgrade
  if (hel_sel)
    ui_c.purchase_time += dt;
  if (!hel_sel)
    ui_c.purchase_time -= dt;
  ui_c.purchase_time = glm::clamp(ui_c.purchase_time, 0.0f, ui_c.purchase_time_max);
  if (hel_sel && ui_c.state.active != nullptr && ui_c.purchase_time >= ui_c.purchase_time_max) {
    do_purchase = true;
    ui_c.purchase_time = 0.0f;
  }

  // back pressed
  if (do_back) {
    back_to_main_menu(r, ui_c);
    return;
  }

  // always be selecting something
  if (ui_c.state.active == nullptr)
    ui_c.state.active = ui_c.state.cells[0];

  const auto viewport_tl = ImVec2((float)ri_c.viewport_pos.x, (float)ri_c.viewport_pos.y);
  const auto viewport_wh = ImVec2((float)ri_c.viewport_size_render_at.x, (float)ri_c.viewport_size_render_at.y);
  const auto viewport_wh_half = ImVec2(viewport_wh.x * 0.5f, viewport_wh.y * 0.5f);

  const float button_size_x = 72.0f;
  const int buttons_per_row = 5;
  const float button_offset = 6.0f; // offset from left
  const float button_inbetween_space_x = 6.0f;
  const float buttons_w = button_size_x * buttons_per_row;
  const float buttons_gaps = (buttons_per_row - 1) * button_inbetween_space_x;
  const float edge_padding = 2.0f * button_offset;
  const float window_x = buttons_w + buttons_gaps + edge_padding;

  const float header_0_y = 0;
  const float header_1_y = 45;
  const float middle_y = edge_padding + button_size_x;
  const float bar_y = 20;
  const float info_y = 110;
  const float window_y = header_1_y + middle_y + info_y;

  const auto custom_tex_id = search_for_texture_id_by_texture_path(ri_c, "custom.png(GL_NEAREST)")->id;
  const auto custom_im_id = (ImTextureID)(void*)(intptr_t)custom_tex_id;

  ImGui::SetNextWindowSize(ImVec2(window_x, window_y), ImGuiCond_Always);
  ImGui::SetNextWindowPos(viewport_wh_half, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

  imgui_begin("upgrade_menu");
  const ImVec2 ui_tl = ImGui::GetWindowPos();
  const ImVec2 ui_wh = ImGui::GetWindowSize();
  const ImVec2 ui_br = { ui_tl.x + ui_wh.x, ui_tl.y + ui_wh.y };
  auto* draw_list = ImGui::GetWindowDrawList();

  // background
  const auto rounding = 0.0f;
  const auto thickness = 1.0f;
  const auto rect_flags = ImDrawFlags_RoundCornersAll;
  draw_list->AddRectFilled(ui_tl, ui_br, im_window_bg_col, rounding);
  draw_list->AddRect(ui_tl, ui_br, im_window_border_col, rounding, ImDrawFlags_RoundCornersAll, thickness);

  // fonts
  auto* fingerpaint_font = get_fingerpaint_font(r);
  const auto font_header_size = (float)FontSizes::SIZE_20;
  auto* text_font = get_inter_font(r);
  const auto font_text_size = (float)FontSizes::SIZE_16;

  // header and dividers
  ImVec2 header_0_tl = { ui_tl.x, ui_tl.y };
  ImVec2 header_0_br = { ui_tl.x + ui_wh.x, header_0_tl.y + header_0_y };
  ImVec2 header_1_tl = { ui_tl.x, header_0_br.y };
  ImVec2 header_1_br = { ui_tl.x + ui_wh.x, header_1_tl.y + header_1_y };
  ImVec2 header_1_wh = calc_wh(header_1_tl, header_1_br);
  ImVec2 info_tl = { ui_tl.x, ui_br.y - info_y };
  ImVec2 info_br = { ui_tl.x + ui_wh.x, ui_br.y };
  ImVec2 info_wh = calc_wh(info_tl, info_br);
  ImVec2 grid_tl = { ui_tl.x, header_1_br.y };
  ImVec2 grid_br = info_br;

  // draw header0 (a tagline)
  // auto tagline_text = "Get good - and upgrade!";
  // draw_list->AddText(fingerpaint_font, font_header_size, header_0_tl, im_greenish, tagline_text);

  // draw header1 (the rows)
  draw_list->AddRect(header_1_tl, header_1_br, im_window_border_col, rounding, ImDrawFlags_RoundCornersAll, thickness);
  draw_header_row(r, ui_c, header_1_tl, do_act, dt);
  draw_moneybag(r, header_1_tl, header_1_wh, text_font, font_text_size, gold_c.amount);

  // draw grid cells
  {
    draw_list->AddRect(grid_tl, grid_br, im_window_border_col, rounding, ImDrawFlags_RoundCornersAll, thickness);

    std::shared_ptr<Cell> selected_header_base = ui_c.active_header;
    if (selected_header_base != nullptr) {
      // just display which header is selected
      const std::string name = selected_header_base.get()->name;
      // draw_list->AddText(grid_tl, im_text_col, name.c_str());

      HeaderCell* selected_header = (HeaderCell*)selected_header_base.get();
      std::vector<std::shared_ptr<Cell>> gridcells = selected_header->gridcells;
      ImVec2 grid_pos = grid_tl;

      // add some padding
      grid_pos.x += button_offset;
      grid_pos.y += button_offset;

      // hack: the cursor seems to be a bit off without this
      const float offset = 14;
      // ImGui::Begin("Debug");
      // imgui_draw_float("offset", offset);
      // ImGui::End();

      for (const auto& cell : gridcells) {
        GridCell* gridcell = (GridCell*)cell.get();

        bool active = cell == ui_c.state.active;
        auto colour = active ? im_greenish : im_text_col;

        // draw_list->AddText(grid_pos, colour, gridcell->name.c_str());
        draw_list->AddRect(grid_pos, { grid_pos.x + button_size_x, grid_pos.y + button_size_x }, colour);

        // draw a cursor on the active cell
        const float cursor_size = ui_c.cursor_c.cursor_size;
        auto pos_tl = grid_pos;
        pos_tl.x += 0.5f * button_size_x - offset;
        if (active)
          draw_cursor(r, ui_c.cursor_c, pos_tl, dt);

        const float icon_sprite = 32;
        const float lock_size = 16;
        const auto draw_icon = [&](std::string key) {
          const auto icon_key = to_upper(key);
          const auto [image_icon_tl, image_icon_br] = convert_sprite_to_uv(r, icon_key);
          auto icon_pos = grid_pos;
          icon_pos.x += 0.5f * (button_size_x - icon_sprite);
          icon_pos.y += 0.5f * (button_size_x - icon_sprite);
          ImGui::SetCursorScreenPos(icon_pos);
          ImGui::Image(custom_im_id, { icon_sprite, icon_sprite }, image_icon_tl, image_icon_br);
        };
        const auto draw_lock_icon = [&](std::string key) {
          const auto ondisk_opt = savefile_get_key(r, key);
          const auto purchased = ondisk_opt.has_value();
          if (!purchased) {
            const auto [lock_tl, lock_br] = convert_sprite_to_uv(r, "ICON_LOCK_32");
            auto lock_pos = grid_pos;
            lock_pos.x += 0.0f * (button_size_x - lock_size);
            lock_pos.y += 1.0f * (button_size_x - lock_size);
            ImGui::SetCursorScreenPos(lock_pos);
            ImGui::Image(custom_im_id, { lock_size, lock_size }, lock_tl, lock_br);
          }
        };

        if (gridcell->type == GridCellType::HULL) {
          const auto& hulls_c = get_first_component<SINGLE_Hulls>(r);
          const auto& hull = hulls_c.hulls[gridcell->index];
          draw_icon("ICON_" + to_upper(hull.key) + "_32");
          draw_lock_icon(hull.key);
        }
        if (gridcell->type == GridCellType::WEAPON) {
          const auto& weapons_c = get_first_component<SINGLE_Weapons>(r);
          const auto& weapon = weapons_c.weapons[gridcell->index];
          draw_icon("ICON_" + to_upper(weapon.key) + "_32");
          draw_lock_icon(weapon.key);
        }
        if (gridcell->type == GridCellType::STAT) {
          const auto idx = gridcell->index;
          const auto stat_enum = magic_enum::enum_cast<UpgradeableStat>(idx).value();
          const auto stat_str = std::string(magic_enum::enum_name<UpgradeableStat>(stat_enum));
          draw_icon("ICON_" + stat_str + "_CENTERED");
          draw_lock_icon(stat_str);
        }
        grid_pos.x += button_size_x + button_inbetween_space_x;
      }
    }
  }

  // bottom info section
  {
    draw_list->AddRect(info_tl, info_br, im_window_border_col, rounding, ImDrawFlags_RoundCornersAll, thickness);

    auto display_info = [&](const auto& gc) {
      ImGui::PushFont(text_font, font_text_size);
      auto text = gc->name;
      auto text_wh = ImGui::CalcTextSize(text.c_str());
      ImGui::PopFont();

      // factor int the purchase bar when centering to make it visually look better
      auto adj_wh = info_wh;
      adj_wh.y -= bar_y;

      // show the item name.
      auto center = calc_center(info_tl, adj_wh);
      auto text_pos = center;
      text_pos.x -= 0.5f * text_wh.x;
      text_pos.y -= 0.5f * text_wh.y;
      draw_list->AddText(text_font, font_text_size, text_pos, im_text_col, text.c_str());

      // Show the cost.
      const auto key = get_gridcell_item_key(r, gc);
      const auto cost = get_item_key_cost(r, key);
      ImVec2 moneybag_tl = center;
      moneybag_tl.y = text_pos.y;
      moneybag_tl.y += font_text_size;
      moneybag_tl.x -= 16;
      draw_moneybag(r, moneybag_tl, { 32, 16 }, text_font, font_text_size, cost);

      // only draw a purchase bar (if not purchased...)
      const auto ondisk_opt = savefile_get_key(r, key);
      const auto show_purchasebar = !ondisk_opt.has_value();
      if (show_purchasebar) {
        const auto purchasebar_tl = ImVec2(info_tl.x + button_offset, info_br.y - 6.0f - bar_y);
        const auto purchasebar_br = ImVec2(info_br.x - button_offset, info_br.y - 6.0f);
        const auto my_player_col = default_player_colours[0];
        const float percent = ui_c.purchase_time / ui_c.purchase_time_max;
        draw_purchasebar(r, purchasebar_tl, purchasebar_br, percent, "Purchase", my_player_col);
      } else {
        auto purchased_text = "Purchased."s;
        const auto purchasebar_tl = ImVec2(info_tl.x + button_offset, info_br.y - 6.0f - bar_y);
        draw_list->AddText(text_font, font_text_size, purchasebar_tl, im_text_col, purchased_text.c_str());
      }
    };

    // display the selected gridcell
    Cell* active_cell = ui_c.state.active.get();
    if (auto gc = dynamic_cast<GridCell*>(active_cell))
      display_info(gc);

    // you've selected a header; display the headers first gridcell
    if (auto hc = dynamic_cast<HeaderCell*>(active_cell)) {
      Cell* gcell = hc->gridcells[0].get();
      GridCell* gc = dynamic_cast<GridCell*>(gcell);
      display_info(gc);
    }
  }

  ImGui::End();

  if (do_purchase) {
    const auto& cell = ui_c.state.active;
    if (auto hc = dynamic_cast<HeaderCell*>(cell.get()))
      return; // cant buy a header cell

    auto* gc = dynamic_cast<GridCell*>(cell.get());
    auto key = get_gridcell_item_key(r, gc);
    auto cost = get_item_key_cost(r, key);

    if (cost > gold_c.amount)
      return; // not enough gold; thats fine.

    // Buy the thing.
    gold_c.amount -= cost;
    savefile_put_key(r, "GOLD_AMOUNT", gold_c.amount);
    savefile_put_key(r, key, 1);
    savefile_save_disk(r);

    // play some audio.
    static engine::RandomState audio_rnd(0);
    const int rnd_audio = engine::rand_det_s(audio_rnd.rng, 1, 7);
    create_empty<AudioRequestPlayEvent>(r, AudioRequestPlayEvent{ "POSITIVE_0" + std::to_string(rnd_audio) });
  }
}

} // namespace game2d