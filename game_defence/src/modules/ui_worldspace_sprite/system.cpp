#include "modules/ui_worldspace_sprite/system.hpp"

#include "actors.hpp"
#include "components.hpp"
#include "lifecycle/components.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/animation/components.hpp"
#include "modules/ui_worldspace_text/helpers.hpp"
#include "renderer/components.hpp"
#include "renderer/transform.hpp"
#include "sprites/helpers.hpp"

#include <fmt/core.h>

namespace game2d {
using namespace std::literals;

const int space_width = 5;
const int char_width = 12; // with of chars
const int char_space = 10; // space between chars
const int line_height = 10;

std::vector<int>
get_length_of_each_line(const std::vector<std::string>& lines)
{
  std::vector<int> line_length(lines.size());

  int working_line_length = 0;
  for (size_t i = 0; i < lines.size(); i++) {
    const auto& line = lines[i];

    for (const char& ch : line) {

      // handle special cases
      if (ch == ' ') {
        working_line_length += space_width;
        continue;
      }

      // default case
      working_line_length += char_space;
    }

    line_length[i] = working_line_length;
    working_line_length = 0;
  }

  return line_length;
};

void
update_ui_worldspace_sprite_system(entt::registry& r)
{
  const auto reqs = r.view<WorldspaceSpriteRequestComponent>();

  for (const auto& [e, req] : reqs.each()) {
    std::vector<std::string> lines = { req.text };
    if (req.split_text_into_chunks)
      lines = split_string_nearest_space(req.text, req.chunk_length);

    // pre-processing. work out line lengths
    const auto line_lengths = get_length_of_each_line(lines);

    // this is the center of the sprite
    const auto pos = get_position(r, e);
    const auto size = get_size(r, e); // size is the size of the enemy that quipped
    float offset_x = 0;
    float offset_y = 0;

    // glm::vec2 tl{ 0.0f, 0.0f };
    // glm::vec2 br{ 0.0f, 0.0f };

    for (size_t i = 0; i < lines.size(); i++) {
      const auto& line = lines[i];
      const auto& line_len = line_lengths[i];

      for (int ch_idx = 0; ch_idx < line.size(); ch_idx++) {
        const auto& ch = line[ch_idx];

        // handle special cases
        if (ch == ' ') {
          offset_x += space_width;
          continue;
        }

        // handle regular cases
        bool allowed_symbol = std::isalpha(ch);
        allowed_symbol |= (bool)std::isdigit(static_cast<unsigned char>(ch));
        allowed_symbol |= ch == '!';
        allowed_symbol |= ch == '?';
        allowed_symbol |= ch == ':';
        allowed_symbol |= ch == '.';
        allowed_symbol |= ch == '%';

        if (!allowed_symbol) {
          fmt::println("unhandled symbol: {}", ch);
          continue;
        }

        const auto sprite_e = create_transform(r);
        r.get<TagComponent>(sprite_e).tag = "empty_with_transform:QuipText";

        // work out the position of the letter.
        glm::vec2 base_position = pos;
        base_position.x -= line_len / 2.0f; // center align the line
        base_position.x += offset_x;
        base_position.y += offset_y;
        base_position.y -= (size.y / 2.0f) + ((lines.size() - 1) * line_height); // put this above the quipper? quippee?

        // Workout background border
        // if (i == 0 && ch_idx == 0)
        //   tl = { base_position.x - char_width / 2.0f, base_position.y - char_width / 2.0f };
        // if (i == lines.size() - 1 && ch_idx == line.size() - 1)
        //   br = { base_position.x + char_width / 2.0f, base_position.y + char_width / 2.0f };

        set_position(r, sprite_e, base_position);
        r.get<TransformComponent>(sprite_e).position.z = 5; // set text in front
        set_size(r, sprite_e, { char_width, char_width });

        WiggleUpAndDown wig;
        wig.base_position = base_position;
        wig.offset = base_position.x; // offset the wiggle of each letter slightly
        wig.amplitude = 1.0;
        r.emplace<WiggleUpAndDown>(sprite_e, wig);
        r.emplace<EntityTimedLifecycle>(sprite_e, 4 * 1000);

        // convert char to sprite index...
        const char upper_ch = std::toupper(static_cast<unsigned char>(ch));
        const std::string spritemap_label = "TEXT_"s + upper_ch;
        set_sprite(r, sprite_e, spritemap_label);

        offset_x += char_space;
      }

      offset_x = 0;
      offset_y += line_height;
    }

    // create a background sprite to increase contrast
    // const auto background_size = glm::vec2(glm::abs(br.x - tl.x), glm::abs(br.y - tl.y));
    // const auto background_center = glm::vec2((br.x + tl.x) / 2.0f, (br.y + tl.y) / 2.0f);
    // const auto background_e = create_gameplay(r, EntityType::empty_with_transform);
    // set_position(r, background_e, background_center);
    // r.get<TransformComponent>(background_e).position.z = 4; // set text in front
    // set_size(r, background_e, size);
    // set_colour(r, background_e, { 0.0f, 0.0f, 0.0f, 1.0f });
    // WiggleUpAndDown wig;
    // wig.base_position = background_center;
    // wig.offset = 0; // offset the wiggle of each letter slightly
    // wig.amplitude = 1.0;
    // r.emplace<WiggleUpAndDown>(background_e, wig);
    // r.emplace<EntityTimedLifecycle>(background_e, 4 * 1000);
  }

  // processed all requests
  r.destroy(reqs.begin(), reqs.end());
}

} // namespace game2d