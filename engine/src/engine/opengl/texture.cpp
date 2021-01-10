
// header
#include "engine/opengl/texture.hpp"

// c lib headers
#include <assert.h>

// c++ standard library headers
#include <iostream>

// other lib headers
#include <stb_image.h>

namespace fightingengine {

void
Texture2D::load_texture_from_file(const std::string& full_path)
{
  glGenTextures(1, &this->id);

  // Stb Load Texture
  int width, height, nr_components;
  unsigned char* data = stbi_load(full_path.c_str(), &width, &height, &nr_components, 0);

  // Check Stb texture loaded correctly
  if (!data) {
    printf("FAILED TO LOAD TEXTURE: %s", full_path.c_str());
    exit(1); // note, probs shouldn't do this - fine for dev for myself
  }

  { // Populate OpenGL information
    GLenum format;
    if (nr_components == 1)
      format = GL_RED;
    else if (nr_components == 3)
      format = GL_RGB;
    else if (nr_components == 4)
      format = GL_RGBA;

    assert(Target == GL_TEXTURE_2D);
    bind();

    glTexImage2D(Target, 0, InternalFormat, width, height, 0, format, Type, data);
    glGenerateMipmap(Target);

    glTexParameteri(Target, GL_TEXTURE_MIN_FILTER, FilterMin);
    glTexParameteri(Target, GL_TEXTURE_MAG_FILTER, FilterMax);
    glTexParameteri(Target, GL_TEXTURE_WRAP_S, WrapS);
    glTexParameteri(Target, GL_TEXTURE_WRAP_T, WrapT);
  }

  // Free Stb image Data
  stbi_image_free(data);
}

void
Texture2D::bind(int unit) const
{
  if (unit >= 0)
    glActiveTexture(GL_TEXTURE0 + unit);
  glBindTexture(Target, this->id);
}

void
Texture2D::unbind()
{
  glBindTexture(Target, 0);
}

} // namespace fightingengine