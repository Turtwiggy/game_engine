#include "graphics/renderer_api.h"

#include "graphics/opengl/opengl_renderer_api.h"

namespace fightinggame {

	renderer_api::API renderer_api::s_API = renderer_api::API::OpenGL;

	std::unique_ptr<renderer_api> renderer_api::create()
	{
		switch (s_API)
		{
		case renderer_api::API::None:    assert(false); /*"RendererAPI::None is currently not supported!" */ return nullptr;
        case renderer_api::API::OpenGL:  return std::make_unique<opengl_renderer_api>();
		}

		assert(false); //unknown renderer api
		return nullptr;
	}
}
