
// header
#include "engine/opengl/texture.hpp"

// fightingengine headers
#include "engine/opengl/framebuffer.hpp"
#include "engine/opengl/render_command.hpp"
#include "engine/opengl/util.hpp"

// other lib headers
#include <GL/glew.h>
#include <stb_image.h>

// c++ standard library headers
#include <iostream>

namespace engine {

StbLoadedTexture
load_texture(const int textureUnit, const std::string& path)
{
  int width, height, nrComponents;
  unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
  StbLoadedTexture result;
  result.width = width;
  result.height = height;
  result.nr_components = nrComponents;
  result.data = data;
  result.texture_unit = textureUnit;
  result.path = path;
  return result;
}

unsigned int
bind_stb_loaded_texture(StbLoadedTexture& texture)
{
  unsigned int textureID;
  glGenTextures(1, &textureID);
  int texture_unit = texture.texture_unit;
  int width = texture.width;
  int height = texture.height;
  int nr_components = texture.nr_components;
  unsigned char* data = texture.data;

  // Check Stb texture loaded correctly
  if (!data) {
    std::cout << "FAILED TO LOAD TEXTURE: " << texture.path << std::endl;
    std::cerr << stbi_failure_reason() << std::endl;
    stbi_image_free(data);
    exit(1); // note, probs shouldn't do this - fine for dev for myself
  }

  std::cout << "binding " << texture.path << " to " << texture.texture_unit << std::endl;

  // activate the texture unit first before binding texture
  glActiveTexture(GL_TEXTURE0 + texture_unit);

  GLenum format;
  if (nr_components == 1) {
    format = GL_RED;
  } else if (nr_components == 3) {
    format = GL_RGB;
  } else if (nr_components == 4) {
    format = GL_RGBA;
  }

  glBindTexture(GL_TEXTURE_2D, textureID);
  glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  bool smooth = true;
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, smooth ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, smooth ? GL_LINEAR : GL_NEAREST);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  stbi_image_free(data);

  glBindTexture(GL_TEXTURE_2D, textureID);

  return textureID;
}

//
// Util Functions
//

void
bind_tex(const int id, const int unit)
{
  if (unit >= 0)
    glActiveTexture(GL_TEXTURE0 + unit);
  glBindTexture(GL_TEXTURE_2D, id);
}

void
unbind_tex()
{
  glBindTexture(GL_TEXTURE_2D, 0);
}

unsigned int
create_texture(glm::ivec2 size, int tex_slot, unsigned int framebuffer_id)
{
  Framebuffer::bind_fbo(framebuffer_id);
  RenderCommand::set_viewport(0, 0, static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y));
  RenderCommand::set_depth_testing(false);
  unsigned int tex_id;
  glGenTextures(1, &tex_id);
  std::cout << "creating texture (" << tex_id << "), and binding to slot: " << tex_slot << std::endl;
  glActiveTexture(GL_TEXTURE0 + tex_slot); // activate the texture unit first before binding texture
  glBindTexture(GL_TEXTURE_2D, tex_id);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // attach it to the currently bound framebuffer object
  {
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_id, 0);
    // tell opengl which colour attachments we'll use of this framebuffer
    unsigned int attachments[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, attachments);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      std::cerr << "(FBO: main_scene) ERROR: Framebuffer not complete!" << std::endl;
      exit(1);
    }
    Framebuffer::default_fbo();
  }
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

} // namespace engine