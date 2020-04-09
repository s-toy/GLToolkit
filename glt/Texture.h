#pragma once
#include <string>
#include <vector>
#include <GLAD/glad.h>
#include "Export.h"

namespace glt
{
	class GLT_DECLSPEC CTexture
	{
	public:
		CTexture() = default;
		virtual ~CTexture() = default;

		virtual void bindV(unsigned int vBindPoint) const = 0;
		virtual void unbindV() const = 0;

		void setTextureName(const std::string& vName) { m_TextureName = vName; }

		const std::string& getFilePath() const { return m_FilePath; }
		const std::string& getTextureName() const { return m_TextureName; }

	protected:
		std::string m_FilePath = {};
		std::string m_TextureName = {};
		unsigned int m_ObjectID = 0;
		mutable unsigned int m_BindPoint = 0;
	};

	class CTexture2D : public CTexture
	{
	public:
		void load(const char *vPath, GLint vWrapMode = GL_CLAMP_TO_BORDER, GLint vFilterMode = GL_LINEAR, GLenum vFormat = GL_RGB, bool vFlipVertically = false);
		void create(unsigned int vWidth, unsigned int vHeight, GLint vInternalFormat = GL_RGB32F, GLenum vFormat = GL_RGB, bool vGenerateMipMap = GL_FALSE);

		void bindV(unsigned int vBindPoint) const override;
		void unbindV() const override;
	};

	class CTextureCube : public CTexture
	{
	public:
		void load(const std::vector<std::string>& vFaces, bool vGenerateMipMap);
		void create(int vWidth, int vHeight, bool vGenerateMipMap);

		void bindV(unsigned int vBindPoint) const override;
		void unbindV() const override;
	};
}