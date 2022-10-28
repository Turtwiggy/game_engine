// header
#include "engine/opengl/texture.hpp"

// fightingengine headers
#include "engine/app/io.hpp"
#include "engine/opengl/framebuffer.hpp"
#include "engine/opengl/render_command.hpp"
#include "engine/opengl/util.hpp"

// other lib headers
#if defined(__EMSCRIPTEN__)
#include <SDL2/SDL_opengles2.h>
#else
#include <GL/glew.h>
#endif
#include <stb_image.h>

// std lib
#include <algorithm>
#include <iostream>
#include <string.h>
// #include <thread> // wahh emscripten
#include <utility>

namespace engine {

void
bind_tex(const int id)
{
  glBindTexture(GL_TEXTURE_2D, id);
}

void
unbind_tex()
{
  glBindTexture(GL_TEXTURE_2D, 0);
}

SRGBTexture
load_texture_srgb(const int tex_unit, const std::string& path)
{
  int width = 0;
  int height = 0;
  int nr_components = 0;
  unsigned char* data = stbi_load(path.c_str(), &width, &height, &nr_components, 0);

  // Check Stb texture loaded correctly
  if (!data) {
    std::cout << "(error) failed to load texture:: " << path << "\n";
    std::cerr << stbi_failure_reason() << "\n";
    stbi_image_free(data);
    exit(1); // if a texture fails to load, explode!
  }

  SRGBTexture srgb; // stbi assumes srgb
  srgb.width = width;
  srgb.height = height;
  srgb.nr_components = nr_components;
  srgb.data = data;
  srgb.texture_unit = tex_unit;
  srgb.path = path;
  return srgb;
}

LinearTexture
load_texture_linear(const int tex_unit, const std::string& path)
{
  SRGBTexture srgb = load_texture_srgb(tex_unit, path);
  int width = srgb.width;
  int height = srgb.height;
  int nr_components = srgb.nr_components;

  LinearTexture linear;
  linear.width = width;
  linear.height = height;
  linear.nr_components = nr_components;
  linear.texture_unit = tex_unit;
  linear.path = path;
  linear.data = new float[width * height * nr_components];

  for (int i = 0; i < srgb.width; i++) {
    for (int j = 0; j < srgb.height; j++) {
      int offset = (i + srgb.width * j) * srgb.nr_components;
      unsigned char* pixel_offset = srgb.data + offset;
      int r = static_cast<int>(pixel_offset[0]);
      int g = static_cast<int>(pixel_offset[1]);
      int b = static_cast<int>(pixel_offset[2]);

      float a = 0.0f;
      if (srgb.nr_components > 3)
        a = static_cast<float>(pixel_offset[3]);

      engine::SRGBColour srgbcol = { r, g, b, a };
      engine::LinearColour lincol = SRGBToLinear(srgbcol);
      linear.data[offset + 0] = lincol.r;
      linear.data[offset + 1] = lincol.g;
      linear.data[offset + 2] = lincol.b;

      if (srgb.nr_components == 4)
        linear.data[offset + 3] = lincol.a;
    }
  }

  stbi_image_free(srgb.data); // free the srgb data
  return linear;
}

std::vector<unsigned int>
load_textures_threaded(std::vector<std::pair<int, std::string>>& textures_to_load)
{
  const auto start = std::chrono::high_resolution_clock::now();
  log_time_since("(Threaded) loading textures... ", start);

  std::vector<unsigned int> texture_ids;

  {
// Try and work out threads on emscrtipten
#if !defined(__EMSCRIPTEN__)
    std::vector<std::thread> threads;
#endif
    std::vector<LinearTexture> loaded_textures(textures_to_load.size());

    for (int i = 0; const auto& texture : textures_to_load) {
#if !defined(__EMSCRIPTEN__)
      threads.emplace_back(
        [&texture, i, &loaded_textures]() { loaded_textures[i] = load_texture_linear(texture.first, texture.second); });
#else
      loaded_textures[i] = load_texture_linear(texture.first, texture.second);
#endif
      i++;
    }

#if !defined(__EMSCRIPTEN__)
    for (auto& thread : threads)
      thread.join();
#endif

    // sort by texture unit
    std::sort(loaded_textures.begin(), loaded_textures.end(), [](LinearTexture a, LinearTexture b) {
      return a.texture_unit < b.texture_unit;
    });

    for (LinearTexture& l : loaded_textures) {
      unsigned int id = bind_linear_texture(l);
      texture_ids.push_back(id);
    }
  }

  log_time_since("(End Threaded) textures loaded took:", start);
  return texture_ids;
}

unsigned int
bind_linear_texture(const LinearTexture& texture)
{
  int tex_unit = texture.texture_unit;
  int width = texture.width;
  int height = texture.height;
  int nr_components = texture.nr_components;
  float* data = texture.data;

  unsigned int texture_id;
  glGenTextures(1, &texture_id);

  GLenum format = GL_RGB;
  if (nr_components == 1)
    format = GL_RED;
  else if (nr_components == 3)
    format = GL_RGB;
  else if (nr_components == 4)
    format = GL_RGBA;

  glActiveTexture(GL_TEXTURE0 + tex_unit);
  glBindTexture(GL_TEXTURE_2D, texture_id);
  glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_FLOAT, data);
  glGenerateMipmap(GL_TEXTURE_2D);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  stbi_image_free(data); // no longer need the texture data
  unbind_tex();
  return texture_id;
}

void
update_bound_texture_size(const glm::ivec2 size)
{
  if (size.x <= 0 || size.y <= 0) {
    std::cerr << "(update_bound_texture_size) ERROR: Invalid resize for texture"
              << "\n";
    return;
  }
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0, GL_RGB, GL_FLOAT, NULL);
};

void
new_texture_to_fbo(unsigned int& out_fbo_id, int& out_tex_id, const int tex_unit, glm::ivec2 size)
{
  glActiveTexture(GL_TEXTURE0 + tex_unit);

  unsigned int fbo_id = Framebuffer::create_fbo();
  Framebuffer::bind_fbo(fbo_id);
  RenderCommand::set_viewport(0, 0, size.x, size.y);

  unsigned int tex_id;
  glGenTextures(1, &tex_id);
  glBindTexture(GL_TEXTURE_2D, tex_id);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0, GL_RGB, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_id, 0);

  // tell opengl which colour attachments we'll use of this framebuffer
  unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0 };
  glDrawBuffers(1, attachments);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cerr << "(FBO: main_scene) ERROR: Framebuffer not complete!"
              << "\n";
    exit(1);
  }

  Framebuffer::default_fbo();
  out_fbo_id = fbo_id;
  out_tex_id = tex_id;
};

} // namespace engine