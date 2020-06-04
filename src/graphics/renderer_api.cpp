#include "graphics/renderer_api.h"

#include "graphics/opengl/opengl_renderer_api.h"
#include "base.h"

namespace fightinggame {

	renderer_api::API renderer_api::s_API = renderer_api::API::OpenGL;

	Scope<renderer_api> renderer_api::Create()
	{
		switch (s_API)
		{
		case renderer_api::API::None:    assert(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case renderer_api::API::OpenGL:  return CreateScope<opengl_renderer_api>();
		}

		assert(false, "Unknown renderer API");
		return nullptr;
	}
}