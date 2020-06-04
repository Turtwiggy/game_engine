
#include "graphics/vertex_buffer.h"
#include "graphics/renderer.h"

#include "graphics/opengl/opengl_vertex_buffer.h"

namespace fightinggame {

	Ref<vertex_buffer> vertex_buffer::Create(uint32_t size)
	{
		switch (renderer::get_api())
		{
		case renderer_api::API::None:    assert(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case renderer_api::API::OpenGL:  return CreateRef<opengl_vertex_buffer>(size);
		}

		assert(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<vertex_buffer> vertex_buffer::Create(float* vertices, uint32_t size)
	{
		switch (renderer::get_api())
		{
		case renderer_api::API::None:    assert(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case renderer_api::API::OpenGL:  return CreateRef<opengl_vertex_buffer>(vertices, size);
		}

		assert(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<index_buffer> index_buffer::Create(uint32_t* indices, uint32_t size)
	{
		switch (renderer::get_api())
		{
		case renderer_api::API::None:    assert(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case renderer_api::API::OpenGL:  return CreateRef<opengl_index_buffer>(indices, size);
		}

		assert(false, "Unknown RendererAPI!");
		return nullptr;
	}

}