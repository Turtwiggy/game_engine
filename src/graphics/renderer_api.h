#pragma once

#include <glm/glm.hpp>

#include "base.h"
#include "graphics/vertex_array.h"

namespace fightinggame {

	class renderer_api
	{
	public:
		enum class API
		{
			None = 0, OpenGL = 1
		};
	public:
		virtual void Init() = 0;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
		virtual void SetClearColor(const glm::vec4& color) = 0;
		virtual void Clear() = 0;

		virtual void DrawIndexed(const Ref<vertex_array>& vertexArray, uint32_t indexCount = 0) = 0;

		static API GetAPI() { return s_API; }
		static Scope<renderer_api> Create();
	private:
		static API s_API; //impl in .cpp
	};

}