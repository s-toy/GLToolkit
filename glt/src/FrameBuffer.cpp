#include "FrameBuffer.h"
#include "Texture.h"

using namespace glt;

//********************************************************************
//FUNCTION:
CFrameBuffer::CFrameBuffer(int vWidth, int vHeight, bool vEnableDepthStencilAttachment) : m_Width(vWidth), m_Height(vHeight)
{
	glGenFramebuffers(1, &m_ObjectID);
	glBindFramebuffer(GL_FRAMEBUFFER, m_ObjectID);

	if (vEnableDepthStencilAttachment)
	{
		unsigned int RenderBufferID = 0;
		glGenRenderbuffers(1, &RenderBufferID);
		glBindRenderbuffer(GL_RENDERBUFFER, RenderBufferID);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_Width, vHeight);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RenderBufferID);
	}

	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//********************************************************************
//FUNCTION:
CFrameBuffer::~CFrameBuffer()
{
	glDeleteFramebuffers(1, &m_ObjectID);
}

//********************************************************************
//FUNCTION:
bool __isColorAttachment(EAttachment vAttachment)
{
	if (vAttachment != EAttachment::DEPTH)
		return true;
}

//*********************************************************************
//FUNCTION:
void CFrameBuffer::set(EAttachment vAttachment, std::shared_ptr<CTexture> vTexture)
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_ObjectID);
	glFramebufferTexture2D(GL_FRAMEBUFFER, (GLenum)vAttachment, GL_TEXTURE_2D, vTexture->getObjectID(), 0);

	m_TextureMap[vAttachment] = vTexture;

	//Specify a list of color buffers to be drawn into
	std::vector<GLuint> ColorAttachments;
	for (auto iter = m_TextureMap.begin(); iter != m_TextureMap.end(); ++iter) 
		if(__isColorAttachment(iter->first)) ColorAttachments.push_back((GLuint)iter->first);
	glDrawBuffers(ColorAttachments.size(), ColorAttachments.data());
}

//*********************************************************************
//FUNCTION:
void CFrameBuffer::bind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_ObjectID);
}

//*********************************************************************
//FUNCTION:
void CFrameBuffer::unbind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}