
#include "graphics/vertex_array.h"

#include "graphics/renderer.h"
#include "graphics/opengl/opengl_vertex_array.h"
#include <cassert>

namespace fightinggame {

	Ref<vertex_array> vertex_array::Create()
	{
		switch (Renderer::get_api())
		{
		case renderer_api::API::None:    assert(false); /*, "RendererAPI::None is currently not supported!");*/ return nullptr;
		case renderer_api::API::OpenGL:  return CreateRef<opengl_vertex_array>();
		}

		assert(false); //unknown renderer api
		return nullptr;
	}

}