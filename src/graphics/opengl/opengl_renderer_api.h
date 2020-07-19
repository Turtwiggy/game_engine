
#pragma once

#include "graphics/renderer_api.h"
#include "graphics/opengl/opengl_vertex_array.h"
#include "util/base.h"

namespace fightinggame {

	class opengl_renderer_api : public renderer_api
	{
	public:
		virtual void init() override;
		virtual void set_viewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

		virtual void set_clear_colour(const glm::vec4& color) override;
		virtual void clear() override;

		virtual void draw_indexed(const Ref<vertex_array>& vertexArray, uint32_t indexCount = 0) override;
	};
}
