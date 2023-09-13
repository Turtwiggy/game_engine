#include "cli.hpp"

#include <iostream>
#include <string>

namespace game2d {

void
parse_args(int argc, char* argv[], Cli& out_cli)
{
  for (int i = 0; i < argc; i++) {
    const std::string input = std::string(argv[i]);
    std::cout << "(CLI ARG) " << input << std::endl;

    if (input.compare("--server") == 0) {
      out_cli.server = true;
      std::cout << "starting as server" << std::endl;
    }

    if (input.compare("--headless") == 0) {
      out_cli.headless = true;
      std::cout << "starting as headless" << std::endl;
    }

    std::string identifier{ "--server-port=" };
    std::size_t contains = input.find(identifier);
    if (contains != std::string::npos) {
      std::string port_str = input.substr(identifier.length(), input.length());
      out_cli.server_port = std::stoi(port_str);
      std::cout << "server port identified; " << out_cli.server_port << std::endl;
    }
  }
};

};