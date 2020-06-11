#include "graphics/texture.h"

#include "graphics/renderer.h"
#include "graphics/opengl/opengl_texture.h"

namespace fightinggame {

    Ref<texture2D> texture2D::Create(uint32_t width, uint32_t height)
    {
        switch (renderer::get_api())
        {
        case renderer_api::API::None:    assert(false); /* "RendererAPI::None is currently not supported!");*/ return nullptr;
        case renderer_api::API::OpenGL:  return CreateRef<opengl_texture2D>(width, height);
        }

        assert(false); //"Unknown RendererAPI!");
        return nullptr;
    }

    Ref<texture2D> texture2D::Create(const char* path, const std::string& directory)
    {
        switch (renderer::get_api())
        {
        case renderer_api::API::None:    assert(false); /* "RendererAPI::None is currently not supported!");*/ return nullptr;
        case renderer_api::API::OpenGL:  return CreateRef<opengl_texture2D>(path, directory);
        }

        assert(false); //"Unknown RendererAPI!");
        return nullptr;
    }

}
