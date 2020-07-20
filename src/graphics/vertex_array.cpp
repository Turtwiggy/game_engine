
#include "graphics/vertex_array.h"

#include "graphics/renderer.h"
#include "graphics/opengl/opengl_vertex_array.h"
#include <cassert>

namespace fightinggame {

    std::shared_ptr<vertex_array> vertex_array::Create()
	{
		switch (Renderer::get_api())
		{
		case renderer_api::API::None:    assert(false); /*, "RendererAPI::None is currently not supported!");*/ return nullptr;
        case renderer_api::API::OpenGL:  return std::make_shared<opengl_vertex_array>();
		}

		assert(false); //unknown renderer api
		return nullptr;
	}

}
