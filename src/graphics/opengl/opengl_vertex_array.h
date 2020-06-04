#pragma once

#include "graphics/vertex_array.h"

namespace fightinggame {

	class opengl_vertex_array : public vertex_array
	{
	public:
		opengl_vertex_array();
		virtual ~opengl_vertex_array();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void AddVertexBuffer(const Ref<vertex_buffer>& vertexBuffer) override;
		virtual void SetIndexBuffer(const Ref<index_buffer>& indexBuffer) override;

		virtual const std::vector<Ref<vertex_buffer>>& GetVertexBuffers() const { return m_VertexBuffers; }
		virtual const Ref<index_buffer>& GetIndexBuffer() const { return m_IndexBuffer; }
	private:
		uint32_t m_RendererID;
		uint32_t m_VertexBufferIndex = 0;
		std::vector<Ref<vertex_buffer>> m_VertexBuffers;
		Ref<index_buffer> m_IndexBuffer;
	};

}