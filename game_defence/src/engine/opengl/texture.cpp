#include "engine/opengl/texture.hpp"

#include "engine/app/io.hpp"
#include "engine/deps/opengl.hpp"
#include "engine/opengl/framebuffer.hpp"
#include "engine/opengl/render_command.hpp"
#include "engine/opengl/util.hpp"
using namespace engine; // used for macro

// other lib headers
#include <stb_image.h>
#include <thread>

// std lib
#include <algorithm>
#include <fmt/core.h>
#include <string>
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
    fmt::println("(error) failed to load texture: {}\n reason: {}", path, stbi_failure_reason());
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
  const int width = srgb.width;
  const int height = srgb.height;
  const int nr_components = srgb.nr_components;

  LinearTexture linear;
  linear.width = width;
  linear.height = height;
  linear.nr_components = nr_components;
  linear.texture_unit = tex_unit;
  linear.path = path;
  linear.data.resize(width * height * nr_components);

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

      const SRGBColour srgbcol = { r, g, b, a / 255.0f };
      const LinearColour lincol = SRGBToLinear(srgbcol);
      const float lin_r = lincol.r;
      const float lin_g = lincol.g;
      const float lin_b = lincol.b;

      linear.data[offset + 0] = lin_r;
      linear.data[offset + 1] = lin_g;
      linear.data[offset + 2] = lin_b;

      if (srgb.nr_components == 4)
        linear.data[offset + 3] = lincol.a;
    }
  }

  stbi_image_free(srgb.data); // free the srgb data
  return linear;
};

unsigned int
bind_linear_texture(const LinearTexture& tex)
{
  const int tex_unit = tex.texture_unit;
  const int width = tex.width;
  const int height = tex.height;
  const int nr_components = tex.nr_components;
  const auto& data = tex.data;

  unsigned int texture_id;
  glGenTextures(1, &texture_id);

  GLenum format_a = GL_RGB;
  GLenum format_b = GL_RGB;

  if (nr_components == 1) {
    format_a = GL_RED;
    format_b = GL_RED;
  }
  if (nr_components == 3) {
    format_a = GL_RGB;
    format_b = GL_RGB;
  }
#if defined(__EMSCRIPTEN__)
  else if (nr_components == 4) {
    format_a = GL_RGBA32F;
    format_b = GL_RGBA;
  }
#else
  else if (nr_components == 4) {
    format_a = GL_RGBA;
    format_b = GL_RGBA;
  }
#endif

  glActiveTexture(GL_TEXTURE0 + tex_unit);
  glBindTexture(GL_TEXTURE_2D, texture_id);
  glTexImage2D(GL_TEXTURE_2D, 0, format_a, width, height, 0, format_b, GL_FLOAT, data.data());
  glGenerateMipmap(GL_TEXTURE_2D);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, tex.texture_min_filter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, tex.texture_max_filter);

  unbind_tex();
  return texture_id;
};

} // namespace engine;

void
engine::update_bound_texture_size(const glm::ivec2 size)
{
  if (size.x <= 0 || size.y <= 0) {
    fmt::println("(update_bound_texture_size) ERROR: Invalid resize for texture");
    return;
  }

#if defined(__EMSCRIPTEN__)
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, size.x, size.y, 0, GL_RGBA, GL_FLOAT, NULL);
#else
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_FLOAT, NULL);
#endif
};

std::vector<unsigned int>
add_textures_to_fbo(const glm::ivec2& size, const int num_colour_buffers)
{
  // generate textures
  auto* tex_ids = new unsigned int[num_colour_buffers];
  glGenTextures(num_colour_buffers, tex_ids);

  for (int i = 0; i < num_colour_buffers; i++) {
    const auto tex_id = tex_ids[i];
    glBindTexture(GL_TEXTURE_2D, tex_id);

    // set parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

#if defined(__EMSCRIPTEN__)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, size.x, size.y, 0, GL_RGBA, GL_FLOAT, NULL);
#else
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_FLOAT, NULL);
#endif

    // attach it to the currently bound framebuffer object
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, tex_id, 0);
  }

  // return texture ids
  std::vector<unsigned int> result;
  for (int i = 0; i < num_colour_buffers; i++)
    result.push_back(tex_ids[i]);

  delete[] tex_ids;
  return result;
};

FboResult
engine::new_texture_to_fbo(const int tex_unit, const glm::ivec2& size, const int n_colour_buffers)
{
  glActiveTexture(GL_TEXTURE0 + tex_unit);

  const auto fbo_id = Framebuffer::create_fbo();
  Framebuffer::bind_fbo(fbo_id);
  RenderCommand::set_viewport(0, 0, size.x, size.y);

  const auto tex_ids = add_textures_to_fbo(size, n_colour_buffers);
  CHECK_OPENGL_ERROR(1);
  if (opengl_error1) {
    fmt::println("Error: failed tex_unit: {}", tex_unit);
    exit(1); // explode
  }

  // tell opengl which colour attachments we'll use of this framebuffer
  unsigned int* attachments = new unsigned int[n_colour_buffers];
  for (int i = 0; i < n_colour_buffers; i++)
    attachments[i] = GL_COLOR_ATTACHMENT0 + i;
  glDrawBuffers(n_colour_buffers, attachments);
  delete[] attachments;

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    fmt::println("(FBO: {}) ERROR: Framebuffer not complete!", tex_unit);
    CHECK_OPENGL_ERROR(2);
    exit(1); // explode
  }

  Framebuffer::default_fbo();

  FboResult result;
  result.out_fbo_id = fbo_id;
  result.out_tex_ids = tex_ids;
  return result;
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