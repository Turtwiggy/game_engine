#include "ui_combat_damage_numbers_system.hpp"
#include "ui_combat_damage_numbers_components.hpp"

#include "actors/actor_helpers.hpp"
#include "imgui.h"
#include "modules/combat/components.hpp"
#include "modules/ui_worldspace_text/components.hpp"

#include <algorithm>
#include <fmt/core.h>

namespace game2d {
using namespace std::literals;

void
handle_damage_event_for_ui(entt::registry& r, const DamageEvent& evt)
{
  const auto from_e = evt.from; // previously bullet, now player?
  const auto to_e = evt.to;
  const auto amount = evt.amount;

  // add a new entry to the UI_BufferComponent...
  auto& buffer_c = r.get_or_emplace<UI_BufferComponent>(to_e);
  TimedEntry entry;
  entry.damage = amount;
  entry.time_left_max = 3.0f;
  entry.time_left = entry.time_left_max;
  buffer_c.entries.push_back(entry);

  fmt::println("adding damage event for ui...");
};

void
update_ui_combat_damage_numbers_system(entt::registry& r, const float dt)
{
  const auto& view = r.view<HealthComponent>();
  for (const auto& [e, hp_c] : view.each()) {
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
    // worldspace_ui.flags |= ImGuiWindowFlags_NoBackground;
    worldspace_ui.alpha = 0.3f;

    const auto& style = ImGui::GetStyle();
    const float ui_padding = style.FramePadding.x;
    const int blocks = 4;
    const float size_x = 8.0f;
    const float size_y = 4.0f;
    const float spacing = 2.0f;

    // Calculate the width of the text e.g. "1 3 5 "
    float total_text_width = 0.0f;
    std::string label = "";
    const int max_hits_to_display = 3;
    const size_t n_entries = ui.entries.size();
    const size_t start_idx = (n_entries > max_hits_to_display) ? n_entries - max_hits_to_display : 0;
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

    const auto text_size = ImGui::CalcTextSize(label.c_str());
    const auto blocks_width = (blocks * size_x) + ((blocks - 1) * spacing) + ui_padding * 2.0f;
    worldspace_ui.size = { glm::max(text_size.x, blocks_width), size_y + text_size.y + ui_padding * 2.0f };

    const auto size = get_size(r, e);
    // worldspace_ui.offset.x = worldspace_ui.size.x / 2.0f;
    worldspace_ui.offset.y = -(size.y / 2.0f) - worldspace_ui.size.y / 2.0f; // place ui above entity

    worldspace_ui.layout = [&ui, &hp_c, label, start_idx, size_x, size_y, spacing, blocks_width]() {
      // Draw ui damage numbers
      const size_t n_entries = ui.entries.size();
      if (n_entries > 0) {
        // The last entry will always have the most recent data in it.
        // Display it left-most.

        for (size_t i = start_idx; i < n_entries; i++) {
          const auto& entry = ui.entries[i];

          if (i != start_idx)
            ImGui::SameLine();

          // todo: make colour (of all text) flash if crit
          const float percent = entry.time_left / entry.time_left_max;
          const auto entry_label = std::to_string((int)entry.damage);
          ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, percent), "%s", entry_label.c_str());
        }
      }

      // Pad the ui, so that the health bar doesnt jump...
      if (n_entries == 0) {
        const std::string idle_label = "Idle";

        // centered
        const auto avail = ImGui::GetContentRegionAvail().x;
        const auto& style = ImGui::GetStyle();
        const auto alignment = 0.5f;
        const auto size = ImGui::CalcTextSize(idle_label.c_str()).x + style.FramePadding.x * 2.0f;
        const auto off = (avail - size) * alignment;
        if (off > 0.0f)
          ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

        ImGui::Text("%s", idle_label.c_str());
      }

      // Draw health blocks
      {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        // center the healthbar...
        float available_width = ImGui::GetContentRegionAvail().x;
        float offset_x = (available_width - blocks_width) / 2.0f;
        if (offset_x < 0.0f)
          offset_x = 0.0f;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset_x);
        const ImVec2 pos = ImGui::GetCursorScreenPos(); // Starting position for the first square

        // health variables
        const float cur_hp = hp_c.hp;
        const float max_hp = hp_c.max_hp;
        const float health_per_block = max_hp / blocks;
        const int full_blocks = (int)(cur_hp / health_per_block);

        // Colors
        ImU32 grey_color = IM_COL32(100, 100, 100, 255);  // Grey color for background blocks
        ImU32 red_color = IM_COL32(255, 0, 0, 255);       // Red color for current health blocks
        ImU32 white_color = IM_COL32(255, 255, 255, 255); // White color for recent damage animation

        // Draw grey background blocks (for full health capacity)
        for (int i = 0; i < blocks; i++) {
          const auto top_left = ImVec2(pos.x + i * (size_x + spacing), pos.y);
          const auto bottom_right = ImVec2(top_left.x + size_x, top_left.y + size_y);
          draw_list->AddRectFilled(top_left, bottom_right, grey_color);
        }

        // Draw fully filled red blocks
        for (int i = 0; i < full_blocks; ++i) {
          const auto top_left = ImVec2(pos.x + i * (size_x + spacing), pos.y);
          const auto bottom_right = ImVec2(top_left.x + size_x, top_left.y + size_y);
          draw_list->AddRectFilled(top_left, bottom_right, red_color);
        }

        // Draw partially filled red block if there is remaining health
        const float fill_percentage = fmod(cur_hp, health_per_block) / health_per_block;
        if (fill_percentage > 0.0f && full_blocks < blocks) {
          ImVec2 top_left = ImVec2(pos.x + full_blocks * (size_x + spacing), pos.y);
          ImVec2 bottom_right = ImVec2(top_left.x + size_x * fill_percentage, top_left.y + size_y);
          draw_list->AddRectFilled(top_left, bottom_right, red_color);
        }

        // Work out the total amount of recieved damage
        int total_dmg = 0;
        std::for_each(ui.entries.begin(), ui.entries.end(), [&total_dmg](const TimedEntry& e) { total_dmg += e.damage; });

        // display... per block...
        struct Block
        {
          ImVec2 tl{ 0.0f, 0.0f };
          ImVec2 br{ 0.0f, 0.0f };
        };
        const auto generate_blocks =
          [&health_per_block, &size_x, &size_y, &spacing, &pos](int cur_hp, int damage) -> std::vector<Block> {
          std::vector<Block> flash_blocks;

          int hp = cur_hp + damage; // already taken the damage
          int damage_remaining = damage;

          for (int i = 0; i < blocks && damage_remaining > 0 && hp > 0; ++i) {
            int cur_block_idx = (hp - 1) / health_per_block;
            int block_start_hp = (cur_block_idx + 0) * health_per_block;
            int block_end_hp = (cur_block_idx + 1) * health_per_block;

            // Determine how much of the current block is affected by damage
            int damage_in_block = std::min(hp - block_start_hp, damage_remaining);

            // Calculate the top-left corner of the entire block
            auto base_tl = ImVec2(pos.x + cur_block_idx * (size_x + spacing), pos.y);

            // Calculate top-left for the white section (damage part)
            // Start from the rightmost point of the remaining health within the block
            float start_x = base_tl.x + (hp - block_start_hp - damage_in_block) * (size_x / (float)health_per_block);
            float width = (damage_in_block / (float)health_per_block) * size_x;
            const auto tl = ImVec2(start_x, base_tl.y);
            const auto br = ImVec2(tl.x + width, base_tl.y + size_y);

            flash_blocks.push_back({ tl, br });

            // Subtract the damage we've applied to this block and reduce the health counter
            damage_remaining -= damage_in_block;
            hp -= damage_in_block;
          }

          return flash_blocks;
        };

        const auto gend_blocks = generate_blocks(hp_c.hp, total_dmg);
        for (const auto& block : gend_blocks)
          draw_list->AddRectFilled(block.tl, block.br, white_color);

        // advance the cursor to avoid overlap with other elements
        // ImGui::Dummy(ImVec2(blocks * (size_x + spacing), size_y));
      }

      //
    };
  }
}

} // namespace game2d