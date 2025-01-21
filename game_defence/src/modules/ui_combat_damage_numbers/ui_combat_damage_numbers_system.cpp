#include "ui_combat_damage_numbers_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/map/components.hpp"
#include "engine/maths/grid.hpp"
#include "engine/renderer/transform.hpp"
#include "imgui.h"
#include "modules/combat/components.hpp"
#include "modules/combat_scale_on_hit/helpers.hpp"
#include "modules/system_names/components.hpp"
#include "modules/ui_worldspace_text/components.hpp"
#include "ui_combat_damage_numbers_components.hpp"

#include <SDL2/SDL_log.h>
#include <algorithm>
#include <format>

namespace game2d {
using namespace std::literals;

void
handle_damage_event_for_ui(entt::registry& r, const DamageEvent& evt)
{
  const auto to_e = evt.to;
  const auto damage = calculate_damage_to_take(r, evt.to, evt.amount, evt.type);

  // add a new entry to the UI_BufferComponent...
  auto& buffer_c = r.get_or_emplace<UI_BufferComponent>(to_e);
  TimedEntry entry;
  entry.damage = (float)damage;
  entry.time_left_max = 3.0f;
  entry.time_left = entry.time_left_max;
  buffer_c.entries.push_back(entry);

  SDL_Log("%s", std::format("adding damage event for ui...").c_str());
};

void
update_ui_combat_damage_numbers_system(entt::registry& r, const float dt, const glm::ivec2 mouse_pos)
{
  const auto& view = r.view<HealthComponent, NameComponent, const TransformComponent>();
  for (const auto& [e, hp_c, name_c, transform_c] : view.each()) {
    auto& worldspace_ui = r.get_or_emplace<WorldspaceTextComponent>(e);
    auto& ui = r.get_or_emplace<UI_BufferComponent>(e);

    // tick down the timer
    std::for_each(ui.entries.begin(), ui.entries.end(), [dt](TimedEntry& entry) { entry.time_left -= dt; });

    // remove the out of time entries...
    const auto to_remove = [](const TimedEntry& entry) { return entry.time_left <= 0.0f; };
    ui.entries.erase(std::remove_if(ui.entries.begin(), ui.entries.end(), to_remove), ui.entries.end());

    worldspace_ui.flags = ImGuiWindowFlags_NoDecoration;
    worldspace_ui.flags |= ImGuiWindowFlags_NoDocking;
    worldspace_ui.flags |= ImGuiWindowFlags_NoFocusOnAppearing;
    worldspace_ui.flags |= ImGuiWindowFlags_NoInputs;
    worldspace_ui.flags |= ImGuiWindowFlags_AlwaysAutoResize;
    // worldspace_ui.flags |= ImGuiWindowFlags_NoBackground;
    worldspace_ui.alpha = 0.6f;

    // only display info if hovered
    glm::ivec2 mouse_gp = { -1, -1 };
    const auto map_e = get_first<MapComponent>(r);
    if (map_e != entt::null) {
      const auto& map_c = get_first_component<MapComponent>(r);
      mouse_gp = engine::grid::worldspace_to_grid_space(mouse_pos, map_c.tilesize);
    }
    const auto gp = get_grid_position(r, e);
    const bool hovered = gp == mouse_gp;
    worldspace_ui.display = hovered;

    // Calculate the width of the text e.g. "1 3 5 "
    std::string label = "";
    const int max_hits_to_display = 3;
    const size_t n_entries = ui.entries.size();
    const size_t start_idx = (n_entries > max_hits_to_display) ? n_entries - max_hits_to_display : 0;
    {
      float total_text_width = 0.0f;
      for (size_t i = start_idx; i < n_entries; i++) {

        std::string text = std::to_string((int)ui.entries[i].damage);

        // fix: dont pad only 1 entry with " "
        if (n_entries == 1) {
          total_text_width += ImGui::CalcTextSize(text.c_str()).x;
          label += text;
          break;
        }

        // fix: dont add " " to last word
        if (i != n_entries - 1)
          text += " "s;

        total_text_width += ImGui::CalcTextSize(text.c_str()).x;
        label += text;
      }
    }

    const auto& style = ImGui::GetStyle();
    const float pad_x = style.WindowPadding.x;
    const float pad_y = style.WindowPadding.y;
    const float both_pad_x = pad_x * 2.0f;
    const float both_pad_y = pad_y * 2.0f;

    auto name = name_c.name;
    const auto spacer = name_c.name.find(" ");
    if (spacer != std::string::npos)
      name = name_c.name.substr(0, spacer);

    float header_width = 0;
    float damagenum_width = ImGui::CalcTextSize(label.c_str()).x;
    float name_width = ImGui::CalcTextSize(name.c_str()).x;
    float name_height = ImGui::CalcTextSize(name.c_str()).y;
    header_width = glm::max(header_width, damagenum_width); // damage numbers, e.g. "0 15 2"
    header_width = glm::max(header_width, name_width);      // the name e.g. "Steve"

    worldspace_ui.offset.y = get_size(r, e).y * -0.85f; // place ui above entity

    worldspace_ui.layout = [&ui, &hp_c, name, label, start_idx, header_width](entt::registry& r) {
      // Draw ui damage numbers
      const size_t n_entries = ui.entries.size();
      if (n_entries > 0) {
        // The last entry will always have the most recent data in it.

        for (size_t i = start_idx; i < n_entries; i++) {
          const auto& entry = ui.entries[i];
          if (i != start_idx)
            ImGui::SameLine();

          // todo: make colour (of all text) flash if crit

          // goes from 1.0 to 0.0
          const float percent_linear = entry.time_left / entry.time_left_max;
          const float percent = ease_out(percent_linear);

          const auto entry_label = std::to_string((int)entry.damage);
          ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, percent), "%s", entry_label.c_str());
        }
      }

      // Pad the ui, so that the health bar doesnt jump...
      if (n_entries == 0)
        ImGui::Text("%s", name.c_str());

      // Draw health blocks
      {
        // blocks styling
        constexpr float health_per_block = 2;
        constexpr int blocks_per_line = 10;
        constexpr float size_x = 4.0f;
        constexpr float size_y = 4.0f;
        constexpr float space_x = 2.0f;
        constexpr float space_y = 2.0f;
        const auto grey_color = IM_COL32(100, 100, 100, 255);  // color for background blocks
        const auto red_color = IM_COL32(255, 0, 0, 255);       // color for current health blocks
        const auto white_color = IM_COL32(255, 255, 255, 255); // color for recent damage

        const float cur_hp = (float)hp_c.hp;
        const float max_hp = (float)hp_c.max_hp;
        const float blocks = max_hp / (int)health_per_block;
        const int n_lines = int(blocks / (float)blocks_per_line);
        const int full_blocks = (int)(cur_hp / health_per_block);

        // work out the width of the healthbar
        const auto& style = ImGui::GetStyle();
        const float hp_w = blocks_per_line * (size_x + space_x) - space_x + style.FramePadding.x * 2.0f;
        const float hp_h = n_lines * (size_y + space_y) + style.FramePadding.y * 2.0f;

        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        // Starting position for the first square
        const ImVec2 pos = ImGui::GetCursorScreenPos();
        const auto get_block_line_idx = [](int block_idx) {
          // values: 1-20
          // should return 1-2
          return block_idx / blocks_per_line;
        };

        // Draw grey background blocks (for full health capacity)
        for (int i = 0; i < blocks; i++) {
          const auto mod_idx = i % blocks_per_line; // [0-blocks_per_line-1]
          const auto line_idx = get_block_line_idx(i);
          const auto tl = ImVec2(pos.x + mod_idx * (size_x + space_x), pos.y + line_idx * (size_y + space_y));
          const auto br = ImVec2(tl.x + size_x, tl.y + size_y);
          draw_list->AddRectFilled(tl, br, grey_color);
        }

        // Draw fully filled red blocks
        for (int i = 0; i < full_blocks; ++i) {
          const auto mod_idx = i % blocks_per_line; // [0-blocks_per_line-1]
          const auto line_idx = get_block_line_idx(i);
          const auto tl = ImVec2(pos.x + mod_idx * (size_x + space_x), pos.y + line_idx * (size_y + space_y));
          const auto br = ImVec2(tl.x + size_x, tl.y + size_y);
          draw_list->AddRectFilled(tl, br, red_color);
        }

        // Draw partially filled red block if there is remaining health
        const float fill_percentage = fmod(cur_hp, health_per_block) / health_per_block;
        if (fill_percentage > 0.0f && full_blocks < blocks) {
          const int i = full_blocks;
          const auto mod_idx = i % blocks_per_line; // [0-blocks_per_line-1]
          const auto line_idx = get_block_line_idx(i);
          const auto tl = ImVec2(pos.x + mod_idx * (size_x + space_x), pos.y + line_idx * (size_y + space_y));
          const auto br = ImVec2(tl.x + size_x * fill_percentage, tl.y + size_y);
          draw_list->AddRectFilled(tl, br, red_color);
        }

        // Work out the total amount of recieved damage
        float total_dmg = 0;
        std::for_each(ui.entries.begin(), ui.entries.end(), [&total_dmg](const TimedEntry& e) { total_dmg += e.damage; });

        // display... per block...
        struct Block
        {
          ImVec2 tl{ 0.0f, 0.0f };
          ImVec2 br{ 0.0f, 0.0f };
        };
        const auto generate_blocks = [&](int cur_hp, int damage) -> std::vector<Block> {
          std::vector<Block> flash_blocks;

          int hp = cur_hp + damage; // already taken the damage
          int damage_remaining = damage;

          for (int it = 0; it < int(blocks) && damage_remaining > 0 && hp > 0; ++it) {
            const auto i = int((hp - 1) / health_per_block);
            const auto mod_idx = i % blocks_per_line; // [0-blocks_per_line-1]
            const auto line_idx = get_block_line_idx(i);

            const int block_start_hp = int((i + 0) * health_per_block);
            const int block_end_hp = int((i + 1) * health_per_block);

            // Determine how much of the current block is affected by damage
            const int damage_in_block = std::min(hp - block_start_hp, damage_remaining);
            const int health_in_block_after_damage = hp - block_start_hp - damage_in_block;

            // Calculate the top-left corner of the entire block
            const auto base_tl = ImVec2(pos.x + mod_idx * (size_x + space_x), pos.y + line_idx * (size_y + space_y));

            // Calculate top-left for the white section (damage part)
            // Start from the rightmost point of the remaining health within the block
            const float start_x = base_tl.x + health_in_block_after_damage * (size_x / (float)health_per_block);
            const float width = (damage_in_block / (float)health_per_block) * size_x;
            const auto tl = ImVec2(start_x, base_tl.y);
            const auto br = ImVec2(tl.x + width, base_tl.y + size_y);

            flash_blocks.push_back({ tl, br });

            // Subtract the damage we've applied to this block and reduce the health counter
            damage_remaining -= damage_in_block;
            hp -= damage_in_block;
          }

          return flash_blocks;
        };
        const auto gen_blocks = generate_blocks(hp_c.hp, int(total_dmg));
        for (const auto& block : gen_blocks)
          draw_list->AddRectFilled(block.tl, block.br, white_color);

        // fill out the line with the correct size
        ImGui::Dummy(ImVec2(std::max(hp_w, header_width), hp_h));
      }

      //
    };
  }
}

} // namespace game2d