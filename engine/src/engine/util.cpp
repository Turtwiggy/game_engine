
// header
#include "util.hpp"

// other library headers
#include <iostream>
#ifdef WIN32
#include <Windows.h>
#endif

// engine header
#include "engine/opengl/texture.hpp"

namespace fightingengine {

void
log_time_since(const std::string& label, std::chrono::time_point<std::chrono::high_resolution_clock> start)
{
  const auto x = std::chrono::high_resolution_clock::now();
  const auto y = std::chrono::duration_cast<std::chrono::milliseconds>(x - start).count();
  std::cout << label << y << "ms" << std::endl;
}

void
load_textures_threaded(std::vector<std::pair<int, std::string>>& textures_to_load,
                       const std::chrono::steady_clock::time_point& app_start)
{
  log_time_since("(Threaded) loading textures... ", app_start);
  {
    std::vector<std::thread> threads;
    std::vector<StbLoadedTexture> loaded_textures(textures_to_load.size());

    for (int i = 0; i < textures_to_load.size(); ++i) {
      const std::pair<int, std::string>& tex_to_load = textures_to_load[i];
      threads.emplace_back([&tex_to_load, i, &loaded_textures]() {
        loaded_textures[i] = load_texture(tex_to_load.first, tex_to_load.second);
      });
    }
    for (auto& thread : threads) {
      thread.join();
    }
    for (StbLoadedTexture& l : loaded_textures) {
      bind_stb_loaded_texture(l);
    }
  }
  log_time_since("(End Threaded) textures loaded ", app_start);
}

void
hide_console()
{
#ifdef WIN32
  ::ShowWindow(::GetConsoleWindow(), SW_HIDE); // hide console
#endif
}

}