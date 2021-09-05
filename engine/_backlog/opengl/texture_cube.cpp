
// // header
// #include "engine/opengl/texture_cube.hpp"

// // your project headers
// #ifndef STB_IMAGE_IMPLEMENTATION
// #define STB_IMAGE_IMPLEMENTATION
// #include <stb_image.h>

// namespace fightingengine {

// void
// TextureCube::default_initialize(unsigned int width, unsigned int height, GLenum format, GLenum type, bool mipmap)
// {
//   glGenTextures(1, &ID);

//   FaceWidth = width;
//   FaceHeight = height;
//   Format = format;
//   Type = type;
//   Mipmapping = mipmap;

//   if (type == GL_HALF_FLOAT && format == GL_RGB)
//     InternalFormat = GL_RGB16F;
//   else if (type == GL_FLOAT && format == GL_RGB)
//     InternalFormat = GL_RGB32F;
//   else if (type == GL_HALF_FLOAT && format == GL_RGBA)
//     InternalFormat = GL_RGBA16F;
//   else if (type == GL_FLOAT && format == GL_RGBA)
//     InternalFormat = GL_RGBA32F;

//   bind();

//   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, FilterMin);
//   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, FilterMax);
//   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, WrapS);
//   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, WrapT);
//   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, WrapR);

//   for (int i = 0; i < 6; ++i) {
//     glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, InternalFormat, width, height, 0, format, type, nullptr);
//   }
//   if (mipmap)
//     glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
// }

// void
// TextureCube::generate_face(GLenum face,
//                            unsigned int width,
//                            unsigned int height,
//                            GLenum format,
//                            GLenum type,
//                            unsigned char* data)
// {
//   if (FaceWidth == 0)
//     glGenTextures(1, &ID);

//   FaceWidth = width;
//   FaceHeight = height;
//   Format = format;
//   Type = type;

//   bind();

//   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, FilterMin);
//   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, FilterMax);
//   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, WrapS);
//   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, WrapT);
//   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, WrapR);

//   glTexImage2D(face, 0, format, width, height, 0, format, type, data);
// }

// void
// TextureCube::bind(int slot)
// {
//   if (slot >= 0)
//     glActiveTexture(GL_TEXTURE0 + slot);
//   glBindTexture(GL_TEXTURE_CUBE_MAP, ID);
// }

// void
// TextureCube::unbind()
// {
//   glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
// }

// TextureCube
// load_texture_cube_from_folder(const std::string& folder)
// {
//   //
//   printf("----- Texture Cube from folder -------\n");

//   TextureCube texture_cube;

//   // disable y flip on cubemaps
//   stbi_set_flip_vertically_on_load(false);

//   std::vector<std::string> faces = { "right.jpg", "left.jpg", "top.jpg", "bottom.jpg", "front.jpg", "back.jpg" };
//   for (unsigned int i = 0; i < faces.size(); ++i) {
//     int width, height, nrComponents;

//     std::string path = folder + faces[i];
//     printf("Dir: %s \n", path.c_str());

//     unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);

//     if (data) {
//       GLenum format;
//       if (nrComponents == 3)
//         format = GL_RGB;
//       else
//         format = GL_RGBA;

//       texture_cube.generate_face(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, width, height, format, GL_UNSIGNED_BYTE, data);
//       stbi_image_free(data);
//     } else {
//       printf("!! Cube texture at path: %s failed to load. !! \n", faces[i].c_str());
//       stbi_image_free(data);
//       return texture_cube;
//     }
//   }

//   if (texture_cube.Mipmapping)
//     glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

//   return texture_cube;
// }

// } // namespace fightingengine