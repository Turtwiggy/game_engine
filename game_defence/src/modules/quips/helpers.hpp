#pragma once

#include "engine/io/file.hpp"
#include <entt/entt.hpp>

namespace game2d {

void
load_quips()
{
  // // Load Quips
  // const auto quips = load_file_into_lines("assets/writing/quips.txt");
  // const auto quips_hit = load_file_into_lines("assets/writing/quips_hit.txt");
  // const auto quips_encounter = load_file_into_lines("assets/writing/quips_encounter.txt");
  // SINGLE_QuipsComponent quips_c;

  // quips_c.quips = quips;
  // quips_c.quips_unused = quips;

  // quips_c.quips_encounter = quips_encounter;
  // quips_c.quips_encounter_unused = quips_encounter;

  // quips_c.quips_hit = quips_hit;
  // quips_c.quips_hit_unused = quips_hit;

  // create_empty<SINGLE_QuipsComponent>(r, quips_c);
}

} // namespace game2d