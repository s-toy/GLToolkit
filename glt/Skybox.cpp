#include "Skybox.h"
#include "Texture.h"
#include "ShaderProgram.h"
#include "VertexBuffer.h"
#include "VertexArray.h"
#include "FileLocator.h"

using namespace glt;

const float skyboxVertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

//*********************************************************************************
//FUNCTION:
CSkybox::CSkybox(const std::vector<std::string>& vFaces)
{
	m_pTexture = std::make_shared<CTextureCube>();
	m_pTexture->load(vFaces, false);

	m_pShaderProgram = std::make_shared<CShaderProgram>();

    std::string VertexShaderFilePath = CFileLocator::getInstance()->locateFile("shaders/draw_skybox_vs.glsl");
    std::string FragmentShaderFilePath = CFileLocator::getInstance()->locateFile("shaders/draw_skybox_fs.glsl");
	m_pShaderProgram->addShader(VertexShaderFilePath, EShaderType::VERTEX_SHADER);
	m_pShaderProgram->addShader(FragmentShaderFilePath, EShaderType::FRAGMENT_SHADER);

    m_pVAO = std::make_shared<CVertexArray>();
    m_pVAO->bind();

	m_pVBO = std::make_shared<CVertexBuffer>(skyboxVertices, sizeof(skyboxVertices));
	CVertexArrayLayout VertexArrayLayout;
	VertexArrayLayout.push<float>(3);
    m_pVAO->addBuffer(*m_pVBO, VertexArrayLayout);

#ifdef _DEBUG
    m_pVAO->unbind();
#endif // _DEBUG
}

//*********************************************************************
//FUNCTION:
void CSkybox::_draw(unsigned int vBindPoint) const
{
	m_pShaderProgram->bind();
	m_pTexture->bindV(vBindPoint);
    m_pVAO->bind();

    m_pShaderProgram->updateUniformTexture("uSkyboxTex", m_pTexture);
    glDepthMask(GL_FALSE);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthMask(GL_TRUE);

#ifdef _DEBUG
    m_pVAO->unbind();
	m_pTexture->unbindV();
	m_pShaderProgram->unbind();
#endif // _DEBUG
}