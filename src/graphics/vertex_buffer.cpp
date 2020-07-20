
#include "graphics/vertex_buffer.h"
#include "graphics/renderer.h"

#include "graphics/opengl/opengl_vertex_buffer.h"

namespace fightinggame {

    std::shared_ptr<vertex_buffer> vertex_buffer::Create(uint32_t size)
	{
		switch (Renderer::get_api())
		{
        case renderer_api::API::None:    assert(false); // , "RendererAPI::None is currently not supported!"); return nullptr;
        case renderer_api::API::OpenGL:  return std::make_shared<opengl_vertex_buffer>(size);
		}

		assert(false); //unknown renderer api
		return nullptr;
	}

    std::shared_ptr<vertex_buffer> vertex_buffer::Create(float* vertices, uint32_t size)
	{
		switch (Renderer::get_api())
		{
        case renderer_api::API::None:    assert(false); // "RendererAPI::None is currently not supported!"); return nullptr;
        case renderer_api::API::OpenGL:  return std::make_shared<opengl_vertex_buffer>(vertices, size);
		}

		assert(false); //unknown renderer api
		return nullptr;
	}

    std::shared_ptr<index_buffer> index_buffer::Create(uint32_t* indices, uint32_t size)
	{
		switch (Renderer::get_api())
		{
        case renderer_api::API::None:    assert(false); // , "RendererAPI::None is currently not supported!"); return nullptr;
        case renderer_api::API::OpenGL:  return std::make_shared<opengl_index_buffer>(indices, size);
		}

		assert(false); //unknown renderer api
		return nullptr;
	}

}
