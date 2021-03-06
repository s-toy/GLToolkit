#pragma once
#include <vector>
#include <glad/glad.h>
#include "Export.h"

namespace glt
{
	struct SVertexArrayElement
	{
		unsigned int Type;
		unsigned int Count;
		bool Normalized;

		static unsigned int getSizeOfType(unsigned int vType)
		{
			switch (vType)
			{
			case GL_FLOAT: return sizeof(GLfloat);
			case GL_UNSIGNED_INT: return sizeof(GLuint);
			case GL_INT: return sizeof(GLint);
			case GL_UNSIGNED_BYTE: return sizeof(GLubyte);
			default: _ASSERTE(false);
			}
			return 0;
		}
	};

	class GLT_DECLSPEC CVertexArrayLayout
	{
	public:
		CVertexArrayLayout();
		~CVertexArrayLayout();

		template<class T>
		void push(unsigned int vCount)
		{
			static_assert(false);
		}

		template<>
		void push<float>(unsigned int vCount)
		{
			m_VertexArrayElements.push_back({ GL_FLOAT, vCount, false });
			m_Stride += vCount * SVertexArrayElement::getSizeOfType(GL_FLOAT);
		}

		template<>
		void push<unsigned int>(unsigned int vCount)
		{
			m_VertexArrayElements.push_back({ GL_UNSIGNED_INT, vCount, false });
			m_Stride += vCount * SVertexArrayElement::getSizeOfType(GL_UNSIGNED_INT);
		}

		template<>
		void push<int>(unsigned int vCount)
		{
			m_VertexArrayElements.push_back({GL_INT, vCount, false});
			m_Stride += vCount * SVertexArrayElement::getSizeOfType(GL_INT);
		}

		unsigned int getStride() const { return m_Stride; }
		unsigned int getNumElements() const { return m_VertexArrayElements.size(); }
		const SVertexArrayElement& getElementAt(unsigned int vIndex) const { _ASSERT(vIndex < m_VertexArrayElements.size()); return m_VertexArrayElements[vIndex]; }

	private:
		std::vector<SVertexArrayElement> m_VertexArrayElements;
		unsigned int m_Stride = 0;
	};
}