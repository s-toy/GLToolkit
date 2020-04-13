#pragma once
#include <string>
#include <vector>
#include <GLAD/glad.h>
#include "Export.h"
#include "ShaderProgram.h"

namespace glt
{
	class GLT_DECLSPEC CTexture
	{
	public:
		CTexture();
		virtual ~CTexture();

		virtual void bindV(unsigned int vBindPoint) const = 0;
		virtual void unbindV() const = 0;

		void setTextureName(const std::string& vName) { m_TextureName = vName; }

		const std::string& getFilePath() const { return m_FilePath; }
		const std::string& getTextureName() const { return m_TextureName; }
		unsigned int getBindPoint() const { return m_BindPoint; }
		unsigned int getObjectID() const { return m_ObjectID; }

	protected:
		std::string m_FilePath = {};
		std::string m_TextureName = {};
		unsigned int m_ObjectID = 0;
		mutable unsigned int m_BindPoint = 0;
	};

	class GLT_DECLSPEC CTexture2D : public CTexture
	{
	public:
		void load(const char *vPath, GLint vWrapMode = GL_CLAMP_TO_BORDER, GLint vFilterMode = GL_LINEAR, GLenum vFormat = GL_RGB, bool vFlipVertically = false);
		void createEmpty(unsigned int vWidth, unsigned int vHeight, GLint vInternalFormat = GL_RGB32F, GLenum vFormat = GL_RGB, bool vGenerateMipMap = GL_FALSE);

		void bindV(unsigned int vBindPoint) const override;
		void unbindV() const override;
	};

	class GLT_DECLSPEC CTextureCube : public CTexture
	{
	public:
		void load(const std::vector<std::string>& vFaces, bool vGenerateMipMap);
		void createEmpty(int vWidth, int vHeight, bool vGenerateMipMap);

		void bindV(unsigned int vBindPoint) const override;
		void unbindV() const override;
	};

	class GLT_DECLSPEC CImage2D : public CTexture
	{
	public:
		void createEmpty(int vWidth, int vHeight, GLenum vFormat, unsigned int vBindUnit);

		void bindV(unsigned int vBindPoint) const override;
		void unbindV() const override;

		void clear();

	private:
		GLenum m_Format = 0;
		int m_Width = 0;
		int m_Height = 0;

		std::unique_ptr<CShaderProgram> m_pShaderProgram;
	};
}