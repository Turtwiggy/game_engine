#include "graphics/opengl/opengl_vertex_array.h"

#include <gl/glew.h>

namespace fightinggame {

	static GLenum ShaderDataTypeToOpenGLBaseType(shader_data_type type)
	{
		switch (type)
		{
		case shader_data_type::Float:    return GL_FLOAT;
		case shader_data_type::Float2:   return GL_FLOAT;
		case shader_data_type::Float3:   return GL_FLOAT;
		case shader_data_type::Float4:   return GL_FLOAT;
		case shader_data_type::Mat3:     return GL_FLOAT;
		case shader_data_type::Mat4:     return GL_FLOAT;
		case shader_data_type::Int:      return GL_INT;
		case shader_data_type::Int2:     return GL_INT;
		case shader_data_type::Int3:     return GL_INT;
		case shader_data_type::Int4:     return GL_INT;
		case shader_data_type::Bool:     return GL_BOOL;
		}

		assert(false); // "Unknown ShaderDataType!"
		return 0;
	}

	opengl_vertex_array::opengl_vertex_array()
	{
		glCreateVertexArrays(1, &m_RendererID);
	}

	opengl_vertex_array::~opengl_vertex_array()
	{
		glDeleteVertexArrays(1, &m_RendererID);
	}

	void opengl_vertex_array::Bind() const
	{
		glBindVertexArray(m_RendererID);
	}

	void opengl_vertex_array::Unbind() const
	{
		glBindVertexArray(0);
	}

	void opengl_vertex_array::AddVertexBuffer(const Ref<vertex_buffer>& vertexBuffer)
	{
		assert(vertexBuffer->GetLayout().GetElements().size()); // "Vertex Buffer has no layout!"

		glBindVertexArray(m_RendererID);
		vertexBuffer->Bind();

		const auto& layout = vertexBuffer->GetLayout();
		for (const auto& element : layout)
		{
			switch (element.Type)
			{
			case shader_data_type::Float:
			case shader_data_type::Float2:
			case shader_data_type::Float3:
			case shader_data_type::Float4:
			case shader_data_type::Int:
			case shader_data_type::Int2:
			case shader_data_type::Int3:
			case shader_data_type::Int4:
			case shader_data_type::Bool:
			{
				glEnableVertexAttribArray(m_VertexBufferIndex);
				glVertexAttribPointer(m_VertexBufferIndex,
					element.GetComponentCount(),
					ShaderDataTypeToOpenGLBaseType(element.Type),
					element.Normalized ? GL_TRUE : GL_FALSE,
					layout.GetStride(),
					(const void*)element.Offset);
				m_VertexBufferIndex++;
				break;
			}
			case shader_data_type::Mat3:
			case shader_data_type::Mat4:
			{
				uint8_t count = element.GetComponentCount();
				for (uint8_t i = 0; i < count; i++)
				{
					glEnableVertexAttribArray(m_VertexBufferIndex);
					glVertexAttribPointer(m_VertexBufferIndex,
						count,
						ShaderDataTypeToOpenGLBaseType(element.Type),
						element.Normalized ? GL_TRUE : GL_FALSE,
						layout.GetStride(),
						(const void*)(sizeof(float) * count * i));
					glVertexAttribDivisor(m_VertexBufferIndex, 1);
					m_VertexBufferIndex++;
				}
				break;
			}
			default:
				assert(false); // "Unknown ShaderDataType!"
			}
		}

		m_VertexBuffers.push_back(vertexBuffer);
	}

	void opengl_vertex_array::SetIndexBuffer(const Ref<index_buffer>& indexBuffer)
	{
		glBindVertexArray(m_RendererID);
		indexBuffer->Bind();

		m_IndexBuffer = indexBuffer;
	}

}