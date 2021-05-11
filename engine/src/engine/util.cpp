
// header
#include "util.hpp"

// other library headers
#include <iostream>

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

uint64_t
encode_cantor_pairing_function(int x, int y)
{
  // If you don't want to make a distinction between the pairs (a, b) and (b, a),
  // then sort a and b before applying the pairing function.
  int temp = 0;
  if (y < x) {
    // Swap X and Y
    int temp = x;
    x = y;
    y = temp;
  }

  int64_t p = 0;
  int i = 0;
  while (x || y) {
    p |= ((uint64_t)(x & 1) << i);
    x >>= 1;
    p |= ((uint64_t)(y & 1) << (i + 1));
    y >>= 1;
    i += 2;
  }
  return p;
}

void
decode_cantor_pairing_function(uint64_t p, uint32_t& x, uint32_t& y)
{
  x = 0;
  y = 0;
  int i = 0;
  while (p) {
    x |= ((uint32_t)(p & 1) << i);
    p >>= 1;
    y |= ((uint32_t)(p & 1) << i);
    p >>= 1;
    i++;
  }
}

}