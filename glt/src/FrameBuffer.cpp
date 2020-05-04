#include "FrameBuffer.h"
#include "Texture.h"

using namespace glt;

//********************************************************************
//FUNCTION:
CFrameBuffer::CFrameBuffer(int vWidth, int vHeight) : m_Width(vWidth), m_Height(vHeight)
{
	glGenFramebuffers(1, &m_ObjectID);
	glBindFramebuffer(GL_FRAMEBUFFER, m_ObjectID);

	unsigned int RenderBufferID = 0;
	glGenRenderbuffers(1, &RenderBufferID);
	glBindRenderbuffer(GL_RENDERBUFFER, RenderBufferID);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_Width, vHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RenderBufferID);

	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//********************************************************************
//FUNCTION:
CFrameBuffer::~CFrameBuffer()
{
	glDeleteFramebuffers(1, &m_ObjectID);
}

//*********************************************************************
//FUNCTION:
void CFrameBuffer::set(EAttachment vAttachment, std::shared_ptr<CTexture> vTexture)
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_ObjectID);
	glFramebufferTexture2D(GL_FRAMEBUFFER, (GLenum)vAttachment, GL_TEXTURE_2D, vTexture->getObjectID(), 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	m_TextureMap[vAttachment] = vTexture;
}

//*********************************************************************
//FUNCTION:
void CFrameBuffer::bind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_ObjectID);

	std::vector<GLuint> Attachments;
	for (auto iter = m_TextureMap.begin(); iter != m_TextureMap.end(); ++iter) Attachments.push_back((GLuint)iter->first);
	glDrawBuffers(Attachments.size(), Attachments.data());
}

//*********************************************************************
//FUNCTION:
void CFrameBuffer::unbind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}