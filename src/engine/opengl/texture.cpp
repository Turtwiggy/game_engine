// header
#include "engine/opengl/texture.hpp"

// fightingengine headers
#include "engine/app/io.hpp"
#include "engine/opengl/framebuffer.hpp"
#include "engine/opengl/render_command.hpp"
#include "engine/opengl/util.hpp"

// other lib headers
#include <GL/glew.h>
#include <stb_image.h>

// c++ standard library headers
#include <algorithm>
#include <iostream>
#include <thread>
#include <utility>

void
engine::bind_tex(const unsigned int id)
{
  glBindTexture(GL_TEXTURE_2D, id);
}

namespace engine {

StbLoadedTexture
load_texture(const int tex_unit, const std::string& path)
{
  int width = 0;
  int height = 0;
  int nr_components = 0;
  unsigned char* data = stbi_load(path.c_str(), &width, &height, &nr_components, 0);
  StbLoadedTexture result;
  result.width = width;
  result.height = height;
  result.nr_components = nr_components;
  result.data = data;
  result.texture_unit = tex_unit;
  result.path = path;
  return result;
}

std::vector<unsigned int>
load_textures_threaded(std::vector<std::pair<int, std::string>>& textures_to_load)
{
  const auto start = std::chrono::high_resolution_clock::now();
  log_time_since("(Threaded) loading textures... ", start);

  std::vector<unsigned int> texture_ids;

  {
    std::vector<std::thread> threads;
    std::vector<StbLoadedTexture> loaded_textures(textures_to_load.size());

    for (int i = 0; i < textures_to_load.size(); i++) {
      const std::pair<int, std::string>& tex_to_load = textures_to_load[i];
      threads.emplace_back([&tex_to_load, i, &loaded_textures]() {
        loaded_textures[i] = load_texture(tex_to_load.first, tex_to_load.second);
      });
    }
    for (auto& thread : threads) {
      thread.join();
    }

    // sort by texture unit
    std::sort(loaded_textures.begin(), loaded_textures.end(), [](StbLoadedTexture a, StbLoadedTexture b) {
      return a.texture_unit < b.texture_unit;
    });

    for (StbLoadedTexture& l : loaded_textures) {
      unsigned int id = bind_stb_loaded_texture(l);
      texture_ids.push_back(id);
    }
  }

  log_time_since("(End Threaded) textures loaded took:", start);
  return texture_ids;
}

unsigned int
bind_stb_loaded_texture(StbLoadedTexture& texture)
{
  int tex_unit = texture.texture_unit;
  int width = texture.width;
  int height = texture.height;
  int nr_components = texture.nr_components;
  unsigned char* data = texture.data;

  // Check Stb texture loaded correctly
  if (!data) {
    std::cout << "FAILED TO LOAD TEXTURE: " << texture.path << std::endl;
    std::cerr << stbi_failure_reason() << std::endl;
    stbi_image_free(data);
    exit(1); // note, probs shouldn't do this - fine for dev for myself?
  }

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

  stbi_image_free(data);
  unbind_tex();
  return texture_id;
}

void
unbind_tex()
{
  glBindTexture(GL_TEXTURE_2D, 0);
}

void
update_bound_texture_size(glm::ivec2 size)
{
  if (size.x <= 0 || size.y <= 0) {
    std::cerr << "(update_bound_texture_size) ERROR: Invalid resize for texture" << std::endl;
    return;
  }
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0, GL_RGB, GL_FLOAT, NULL);
};

void
new_texture_to_fbo(unsigned int& out_fbo_id, unsigned int& out_tex_id, const int tex_unit, glm::ivec2 size)
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
    std::cerr << "(FBO: main_scene) ERROR: Framebuffer not complete!" << std::endl;
    exit(1);
  }

  Framebuffer::default_fbo();
  out_fbo_id = fbo_id;
  out_tex_id = tex_id;
};

} // namespace engine