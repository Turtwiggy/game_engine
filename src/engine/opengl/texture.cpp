// header
#include "engine/opengl/texture.hpp"

// fightingengine headers
#include "engine/app/io.hpp"
#include "engine/opengl/framebuffer.hpp"
#include "engine/opengl/render_command.hpp"
#include "engine/opengl/util.hpp"
using namespace engine; // used for macro

// other lib headers
#if defined(__EMSCRIPTEN__)
#include <SDL2/SDL_opengles2.h>
#else
#include <GL/glew.h>
#endif
#include <stb_image.h>
#include <thread>

// std lib
#include <algorithm>
#include <iostream>
#include <string.h>
#include <utility>

namespace engine {

void
bind_tex(const int id)
{
  glBindTexture(GL_TEXTURE_2D, id);
};

void
unbind_tex()
{
  glBindTexture(GL_TEXTURE_2D, 0);
};

SRGBTexture
load_texture_srgb(const int tex_unit, const std::string& path)
{
  int width = 0;
  int height = 0;
  int nr_components = 0;
  unsigned char* data = stbi_load(path.c_str(), &width, &height, &nr_components, 0);
  // data contains [0, 255]

  // Check Stb texture loaded correctly
  if (!data) {
    std::cout << "(error) failed to load texture:: " << path << "\n";
    std::cerr << stbi_failure_reason() << "\n";
    stbi_image_free(data);
    exit(1); // if a texture fails to load, explode!
  }

  engine::SRGBTexture srgb; // stbi assumes srgb
  srgb.width = width;
  srgb.height = height;
  srgb.nr_components = nr_components;
  srgb.data = data;
  srgb.texture_unit = tex_unit;
  srgb.path = path;
  return srgb;
};

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
  // linear.data = new float[width * height * nr_components];
  linear.data = new unsigned char[width * height * nr_components];

  for (int i = 0; i < srgb.width; i++) {
    for (int j = 0; j < srgb.height; j++) {
      int offset = (i + srgb.width * j) * srgb.nr_components;
      unsigned char* pixel_offset = srgb.data + offset;
      int r = static_cast<int>(pixel_offset[0]);
      int g = static_cast<int>(pixel_offset[1]);
      int b = static_cast<int>(pixel_offset[2]);

      int a = 0;
      if (srgb.nr_components > 3)
        a = static_cast<int>(pixel_offset[3]);

      SRGBColour srgbcol = { r, g, b, a / 255.0f };
      LinearColour lincol = SRGBToLinear(srgbcol);
      int lin_r = static_cast<int>(lincol.r * 255.0f);
      int lin_g = static_cast<int>(lincol.g * 255.0f);
      int lin_b = static_cast<int>(lincol.b * 255.0f);

      linear.data[offset + 0] = lin_r;
      linear.data[offset + 1] = lin_g;
      linear.data[offset + 2] = lin_b;

      if (srgb.nr_components == 4)
        linear.data[offset + 3] = static_cast<int>(lincol.a * 255.0f);
    }
  }

  stbi_image_free(srgb.data); // free the srgb data
  return linear;
};

} // namespace engine;

unsigned int
engine::bind_linear_texture(const LinearTexture& texture)
{
  int tex_unit = texture.texture_unit;
  int width = texture.width;
  int height = texture.height;
  int nr_components = texture.nr_components;
  unsigned char* data = texture.data;

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
  glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
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
engine::update_bound_texture_size(const glm::ivec2 size)
{
  if (size.x <= 0 || size.y <= 0) {
    std::cerr << "(update_bound_texture_size) ERROR: Invalid resize for texture\n";
    return;
  }
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
};

void
engine::new_texture_to_fbo(unsigned int& out_fbo_id, int& out_tex_id, const int tex_unit, glm::ivec2 size)
{
  glActiveTexture(GL_TEXTURE0 + tex_unit);

  unsigned int fbo_id = Framebuffer::create_fbo();
  Framebuffer::bind_fbo(fbo_id);
  RenderCommand::set_viewport(0, 0, size.x, size.y);

  unsigned int tex_id;
  glGenTextures(1, &tex_id);
  glBindTexture(GL_TEXTURE_2D, tex_id);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_id, 0);

  // tell opengl which colour attachments we'll use of this framebuffer
  unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0 };
  glDrawBuffers(1, attachments);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cerr << "(FBO: main_scene) ERROR: Framebuffer not complete!\n";
    CHECK_OPENGL_ERROR(1);
    exit(1);
  }

  Framebuffer::default_fbo();
  out_fbo_id = fbo_id;
  out_tex_id = tex_id;
};

std::vector<unsigned int>
engine::load_textures(const std::vector<std::pair<int, std::string>>& textures_to_load)
{
  const auto start = std::chrono::high_resolution_clock::now();
  log_time_since("loading textures... ", start);
  std::vector<unsigned int> texture_ids;

  std::vector<LinearTexture> loaded_textures(textures_to_load.size());
  for (int i = 0; const auto& texture : textures_to_load) {
    loaded_textures[i] = load_texture_linear(texture.first, texture.second);
    i++;
  }

  // sort by texture unit
  std::sort(loaded_textures.begin(), loaded_textures.end(), [](LinearTexture a, LinearTexture b) {
    return a.texture_unit < b.texture_unit;
  });

  for (LinearTexture& l : loaded_textures) {
    unsigned int id = bind_linear_texture(l);
    texture_ids.push_back(id);
  }

  log_time_since("textures loaded took:", start);
  return texture_ids;
}

std::vector<unsigned int>
engine::load_textures_threaded(const std::vector<std::pair<int, std::string>>& textures_to_load)
{
  const auto start = std::chrono::high_resolution_clock::now();
  log_time_since("(Threaded) loading textures... ", start);

  std::vector<unsigned int> texture_ids;

  {
    // Try and work out threads on emscrtipten
    std::vector<std::thread> threads;

    std::vector<LinearTexture> loaded_textures(textures_to_load.size());
    for (int i = 0; const auto& texture : textures_to_load) {
      threads.emplace_back([&texture, i, &loaded_textures]() {
        //
        loaded_textures[i] = load_texture_linear(texture.first, texture.second);
      });
      i++;
    }

    for (auto& thread : threads)
      thread.join();

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