#include "graphics_context.h"
#include "renderer_api.h"

#include "cassert"

namespace fightingengine {

    std::unique_ptr<GraphicsContext> GraphicsContext::Create(void* window)
    {
        switch (Renderer::GetAPI())
        {
        case RendererAPI::API::None:    HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
        case RendererAPI::API::OpenGL:  return std::make_unqiue<OpenGLContext>(static_cast<GLFWwindow*>(window));
        }

        assert(false); // "Unknown RendererAPI!
        return nullptr;
    }

}
