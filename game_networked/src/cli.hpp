#pragma once

namespace game2d {

struct Cli
{
  bool server = false;
  int server_port = 27020;

  // no graphics
  bool headless = false;
};

void
parse_args(int argc, char* argv[], Cli& out_cli);

} // namespace game2d