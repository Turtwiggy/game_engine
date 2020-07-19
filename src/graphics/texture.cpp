
#include "graphics/texture.h"

#include "graphics/renderer.h"
#include "graphics/opengl/opengl_texture.h"

namespace fightinggame {

    Ref<Texture2D> Texture2D::Create(uint32_t width, uint32_t height, std::string unique_name)
    {
        switch (Renderer::get_api())
        {
        case renderer_api::API::None:    assert(false); /* "RendererAPI::None is currently not supported!");*/ return nullptr;
        case renderer_api::API::OpenGL:  return CreateRef<opengl_texture2D>(width, height, unique_name);
        }

        assert(false); //"Unknown RendererAPI!");
        return nullptr;
    }

    Ref<Texture2D> Texture2D::Create(const char* file_name, const std::string& directory)
    {
        switch (Renderer::get_api())
        {
        case renderer_api::API::None:    assert(false); /* "RendererAPI::None is currently not supported!");*/ return nullptr;
        case renderer_api::API::OpenGL:  return CreateRef<opengl_texture2D>(file_name, directory);
        }

        assert(false); //"Unknown RendererAPI!");
        return nullptr;
    }

}
