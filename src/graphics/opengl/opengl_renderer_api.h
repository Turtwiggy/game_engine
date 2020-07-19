
#pragma once

#include "graphics/renderer_api.h"
#include "graphics/opengl/opengl_vertex_array.h"
#include "util/base.h"

namespace fightinggame {

	class opengl_renderer_api : public renderer_api
	{
	public:
		virtual void Init() override;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

		virtual void SetClearColor(const glm::vec4& color) override;
		virtual void Clear() override;

		virtual void DrawIndexed(const Ref<vertex_array>& vertexArray, uint32_t indexCount = 0) override;
	};
}
