#include "graphics/renderer_api.h"

#include "graphics/opengl/opengl_renderer_api.hpp"

namespace fightingengine {

	RendererAPI::API RendererAPI::s_API = RendererAPI::API::OpenGL;

	std::unique_ptr<RendererAPI> RendererAPI::create()
	{
		switch (s_API)
		{
		case RendererAPI::API::None:    assert(false); /*"RendererAPI::None is currently not supported!" */ return nullptr;
        case RendererAPI::API::OpenGL:  return std::make_unique<opengl_renderer_api>();
		}

		assert(false); //unknown renderer api
		return nullptr;
	}
}
