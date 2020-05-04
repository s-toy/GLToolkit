#pragma once
#include <memory>
#include <map>
#include <glad/glad.h>
#include "Export.h"

namespace glt
{
	class CTexture;

	enum class EAttachment : GLenum
	{
		COLOR0 = GL_COLOR_ATTACHMENT0,
		COLOR1 = GL_COLOR_ATTACHMENT1,
		COLOR2 = GL_COLOR_ATTACHMENT2,
		COLOR3 = GL_COLOR_ATTACHMENT3,
		COLOR4 = GL_COLOR_ATTACHMENT4,
		COLOR5 = GL_COLOR_ATTACHMENT5,
		COLOR6 = GL_COLOR_ATTACHMENT6,
		COLOR7 = GL_COLOR_ATTACHMENT7,

		DEPTH = GL_DEPTH_ATTACHMENT
	};

	class GLT_DECLSPEC CFrameBuffer
	{
	public:
		CFrameBuffer(int vWidth, int vHeight);
		~CFrameBuffer();

		void set(EAttachment vAttachment, std::shared_ptr<CTexture> vTexture);
		std::shared_ptr<CTexture> texture(EAttachment vAttachment) { return m_TextureMap[vAttachment]; }

		void bind() const;
		void unbind() const;

	private:
		unsigned int m_ObjectID = 0;
		int m_Width = 0, m_Height = 0;

		std::map<EAttachment, std::shared_ptr<CTexture>> m_TextureMap;
	};
}