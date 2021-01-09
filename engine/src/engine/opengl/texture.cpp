
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
load_texture(const std::string& full_path)
{
  int width, height, nrChannels;
  unsigned char* data = stbi_load(full_path.c_str(), &width, &height, &nrChannels, 0);

  // flip textures on their y coordinate while loading
  // stbi_set_flip_vertically_on_load(false);

  if (!data) {
    printf("FAILED TO LOAD TEXTURE: %s", full_path.c_str());
    exit(1); // note, probs shouldn't do this - fine for dev for myself
  }

  StbLoadedTexture tex;
  tex.width = width;
  tex.height = height;
  tex.nr_components = nrChannels;
  tex.data = data;

  return tex;
}

void
Texture2D::generate(StbLoadedTexture tex)
{
  std::cout << "Texture2D::generate()... " << std::endl;

  if (!tex.data) {
    std::cerr << stbi_failure_reason() << std::endl;
    stbi_image_free(tex.data);
    exit(1);
  }

  glGenTextures(1, &this->id);

  GLenum format;
  if (tex.nr_components == 1)
    format = GL_RED;
  else if (tex.nr_components == 3)
    format = GL_RGB;
  else if (tex.nr_components == 4)
    format = GL_RGBA;

  assert(Target == GL_TEXTURE_2D);
  bind();

  glTexImage2D(this->Target, 0, this->InternalFormat, tex.width, tex.height, 0, format, this->Type, tex.data);
  glTexParameteri(Target, GL_TEXTURE_MIN_FILTER, FilterMin);
  glTexParameteri(Target, GL_TEXTURE_MAG_FILTER, FilterMax);
  glTexParameteri(Target, GL_TEXTURE_WRAP_S, WrapS);
  glTexParameteri(Target, GL_TEXTURE_WRAP_T, WrapT);

  if (Mipmapping)
    glGenerateMipmap(Target);

  // and finally free image data
  stbi_image_free(tex.data);

  unbind();
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