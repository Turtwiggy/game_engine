// header
#include "engine/opengl/texture.hpp"

// fightingengine headers
#include "engine/opengl/framebuffer.hpp"
#include "engine/opengl/render_command.hpp"
#include "engine/opengl/util.hpp"
#include <engine/util.hpp>

// other lib headers
#include <GL/glew.h>
#include <stb_image.h>

// c++ standard library headers
#include <algorithm>
#include <iostream>

namespace engine {

StbLoadedTexture
load_texture(const int tex_unit, const std::string& path)
{
  int width, height, nrComponents;
  unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
  StbLoadedTexture result;
  result.width = width;
  result.height = height;
  result.nr_components = nrComponents;
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

    // sort by texture unit as i am bad at opengl and dont understand
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
  unsigned int tex_id;
  int tex_unit = texture.texture_unit;
  glGenTextures(1, &tex_id);
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

  bind_tex(tex_id, tex_unit);
  {
    GLenum format;
    if (nr_components == 1) {
      format = GL_RED;
    } else if (nr_components == 3) {
      format = GL_RGB;
    } else if (nr_components == 4) {
      format = GL_RGBA;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    bool smooth = false;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, smooth ? GL_LINEAR : GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, smooth ? GL_LINEAR_MIPMAP_LINEAR :
    // GL_NEAREST_MIPMAP_LINEAR); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_image_free(data);
  }
  unbind_tex();
  return tex_id;
}

void
bind_tex(const int id, const int unit)
{
  if (unit >= 0) {
    std::cout << "(texture) id: " << id << " bound to unit: " << unit << std::endl;
    glActiveTexture(GL_TEXTURE0 + unit);
  }
  glBindTexture(GL_TEXTURE_2D, id);
}

void
unbind_tex()
{
  glBindTexture(GL_TEXTURE_2D, 0);
}

unsigned int
create_texture(const glm::ivec2 size, const int tex_unit)
{
  unsigned int tex_id;
  glGenTextures(1, &tex_id);
  bind_tex(tex_id, tex_unit);
  {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }
  unbind_tex();
  return tex_id;
};

void
update_bound_texture_size(glm::ivec2 size)
{
  if (size.x <= 0 || size.y <= 0) {
    std::cerr << "(update_bound_texture_size) ERROR: Invalid resize for texture" << std::endl;
    return;
  }
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
};

void
attach_texture_to_fbo(const int tex_id, const int fbo_id, glm::vec2 size)
{
  Framebuffer::bind_fbo(fbo_id);
  RenderCommand::set_viewport(0, 0, size.x, size.y);
  bind_tex(tex_id);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_id, 0);

  // tell opengl which colour attachments we'll use of this framebuffer
  unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0 };
  glDrawBuffers(1, attachments);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cerr << "(FBO: main_scene) ERROR: Framebuffer not complete!" << std::endl;
    exit(1);
  }

  Framebuffer::default_fbo();
};

} // namespace engine