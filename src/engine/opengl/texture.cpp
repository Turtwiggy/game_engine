#include "pch.hpp"

#include "engine/opengl/texture.hpp"

#include "engine/app/io.hpp"
#include "engine/deps/opengl.hpp"
#include "engine/opengl/framebuffer.hpp"
#include "engine/opengl/render_command.hpp"
#include "engine/opengl/util.hpp"
using namespace engine; // used for macro

// other lib headers
#include <stb_image.h>

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

void
get_format(const int nr_components, GLenum& a, GLenum& b)
{
  GLenum format_a = GL_RGB;
  GLenum format_b = GL_RGB;

  if (nr_components == 1) {
    format_a = GL_RED;
    format_b = GL_RED;
  }

#if defined(__EMSCRIPTEN__)
  if (nr_components == 3) {
    format_a = GL_RGB32F;
    format_b = GL_RGB;
  }
#else
  if (nr_components == 3) {
    format_a = GL_RGB;
    format_b = GL_RGB;
  }
#endif

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
};

engine::SRGBTexture
load_texture(std::string path, const uint32_t tex_unit)
{
  int width = 0;
  int height = 0;
  int nr_components = 0;
  unsigned char* data = stbi_load(path.c_str(), &width, &height, &nr_components, 0);
  if (!data) {
    SDL_Log("%s", std::format("(error) failed to load texture: {}\n reason: {}", path, stbi_failure_reason()).c_str());
    stbi_image_free(data);
    exit(1); // if a texture fails to load, explode!
  }

  if (nr_components != 4) {
    SDL_Log("%s", std::format("(error) texture must be 4 components: {}", path).c_str());
    stbi_image_free(data);
    exit(1);
  }

  GLenum format_a = GL_RGB;
  GLenum format_b = GL_RGB;
  get_format(nr_components, format_a, format_b);

  // Create a OpenGL texture identifier
  uint32_t image_texture;
  glGenTextures(1, &image_texture);
  glActiveTexture(GL_TEXTURE0 + tex_unit);
  glBindTexture(GL_TEXTURE_2D, image_texture);

  // Setup filtering parameters for display
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Upload pixels into texture
  glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  stbi_image_free(data);

#if defined(_DEBUG)
  CHECK_OPENGL_ERROR(6);
#endif

  return engine::SRGBTexture{
    .width = width,
    .height = height,
    .nr_components = nr_components,
    .texture_id = image_texture,
    .texture_unit = tex_unit,
    .path = path,
  };
}

} // namespace engine;

void
engine::update_bound_texture_size(const glm::ivec2 size)
{
  if (size.x <= 0 || size.y <= 0) {
    SDL_Log("%s", std::format("(update_bound_texture_size) ERROR: Invalid resize for texture").c_str());
    return;
  }

#if defined(__EMSCRIPTEN__)
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
#else
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
#endif
};

std::vector<unsigned int>
add_textures_to_fbo(const glm::ivec2& size, const TextureFiltering& f, const int num_colour_buffers)
{
  // generate textures
  std::vector<unsigned int> tex_ids(num_colour_buffers);
  glGenTextures(num_colour_buffers, tex_ids.data());

  for (int i = 0; i < num_colour_buffers; i++) {
    const auto tex_id = tex_ids[i];
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // attach it to the currently bound framebuffer object
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, tex_id, 0);
  }

  // return texture ids
  std::vector<unsigned int> result;
  for (int i = 0; i < num_colour_buffers; i++)
    result.push_back(tex_ids[i]);

  return result;
};

FboResult
engine::new_texture_to_fbo(const int tex_unit, const glm::ivec2& size, const TextureFiltering& f, const int n_colour_buffers)
{
  glActiveTexture(GL_TEXTURE0 + tex_unit);

  const auto fbo_id = Framebuffer::create_fbo();
  Framebuffer::bind_fbo(fbo_id);
  RenderCommand::set_viewport(0, 0, size.x, size.y);

  const auto tex_ids = add_textures_to_fbo(size, f, n_colour_buffers);
  if (CHECK_OPENGL_ERROR(1)) {
    SDL_Log("%s", std::format("Error: failed tex_unit: {}", tex_unit).c_str());
    exit(1); // explode
  }

  // tell opengl which colour attachments we'll use of this framebuffer
  unsigned int* attachments = new unsigned int[n_colour_buffers];
  for (int i = 0; i < n_colour_buffers; i++)
    attachments[i] = GL_COLOR_ATTACHMENT0 + i;
  glDrawBuffers(n_colour_buffers, attachments);
  delete[] attachments;

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    SDL_Log("%s", std::format("(FBO: {}) ERROR: Framebuffer not complete!", tex_unit).c_str());
    CHECK_OPENGL_ERROR(2);
    exit(1); // explode
  }

  Framebuffer::default_fbo();

  FboResult result;
  result.out_fbo_id = fbo_id;
  result.out_tex_ids = tex_ids;
  return result;
};
