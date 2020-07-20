#pragma once

#include "graphics/vertex_array.h"

#include <memory>

namespace fightinggame {

	class opengl_vertex_array : public vertex_array
	{
	public:
		opengl_vertex_array();
		virtual ~opengl_vertex_array();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void AddVertexBuffer(const std::shared_ptr<vertex_buffer>& vertexBuffer) override;
		virtual void SetIndexBuffer(const std::shared_ptr<index_buffer>& indexBuffer) override;

		virtual const std::vector<std::shared_ptr<vertex_buffer>>& GetVertexBuffers() const { return m_VertexBuffers; }
		virtual const std::shared_ptr<index_buffer>& GetIndexBuffer() const { return m_IndexBuffer; }
	private:
		uint32_t m_RendererID;
		uint32_t m_VertexBufferIndex = 0;
		std::vector<std::shared_ptr<vertex_buffer>> m_VertexBuffers;
        std::shared_ptr<index_buffer> m_IndexBuffer;
	};

}
