
// header
#include "engine/opengl/texture.hpp"

// c lib headers
#include <assert.h>

// c++ standard library headers
#include <iostream>

// other lib headers
#include <stb_image.h>

namespace fightingengine {

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

void
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
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  stbi_image_free(data);

  glBindTexture(GL_TEXTURE_2D, textureID);
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

} // namespace fightingengine