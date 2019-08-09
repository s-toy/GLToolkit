#include "Renderer.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "ShaderProgram.h"
#include "Model.h"

using namespace glt;

//***********************************************************************************
//FUNCTION:
#ifdef _DEBUG
static void __glDebugCallback(GLenum vSource, GLenum vType, GLuint vID, GLenum vSeverity, GLsizei vLength, const GLchar *vMessage, const void *vUserParam)
{
	// 忽略一些不重要的错误/警告代码
	if (vID == 131169 || vID == 131185 || vID == 131218 || vID == 131204) return;

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << vID << "): " << vMessage << std::endl;

	switch (vSource)
	{
	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
	} std::cout << std::endl;

	switch (vType)
	{
	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
	} std::cout << std::endl;

	switch (vSeverity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
	} std::cout << std::endl;
	std::cout << std::endl;
}
#endif

//***********************************************************************************************
//FUNCTION:
bool CRenderer::init()
{
	_EARLY_RETURN(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress), "Failed to initialize GLAD.", false);

#ifdef _DEBUG
	GLint Flags; glGetIntegerv(GL_CONTEXT_FLAGS, &Flags);
	if (Flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(__glDebugCallback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}
#endif // DEBUG

	glEnable(GL_DEPTH_TEST);

	m_pCamera = new CCamera;

	return true;
}

//***********************************************************************************************
//FUNCTION:
void CRenderer::destroy()
{
	_SAFE_DELETE(m_pCamera);
}

//***********************************************************************************************
//FUNCTION:
void CRenderer::clear() const
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

//*********************************************************************
//FUNCTION:
void CRenderer::draw(const CVertexArray& vVertexArray, const CIndexBuffer& vIndexBuffer, const CShaderProgram& vShaderProgram) const
{
	vVertexArray.bind();
	vIndexBuffer.bind();
	vShaderProgram.bind();

	__updateShaderUniform(vShaderProgram);
	glDrawElements(GL_TRIANGLES, vIndexBuffer.getCount(), GL_UNSIGNED_INT, nullptr);

#ifdef _DEBUG
	vVertexArray.unbind();
	vIndexBuffer.unbind();
	vShaderProgram.unbind();
#endif // DEBUG
}

//***********************************************************************************************
//FUNCTION:
void CRenderer::draw(const CModel& vModel, const CShaderProgram& vShaderProgram)
{
	vShaderProgram.bind();

	__updateShaderUniform(vShaderProgram);
	auto ModelMatrix = glm::translate(glm::mat4(1.0), vModel.getPosition());
	ModelMatrix = glm::scale(ModelMatrix, vModel.getScale());
	vShaderProgram.updateUniformMat4("uModelMatrix", ModelMatrix);

	vModel.draw(vShaderProgram);

#ifdef _DEBUG
	vShaderProgram.unbind();
#endif
}

//***********************************************************************************************
//FUNCTION:
void CRenderer::__updateShaderUniform(const CShaderProgram& vShaderProgram) const
{
	vShaderProgram.updateUniform3f("uViewPos", m_pCamera->getPosition());
	vShaderProgram.updateUniformMat4("uProjectionMatrix", m_pCamera->getProjectionMatrix());
	vShaderProgram.updateUniformMat4("uViewMatrix", m_pCamera->getViewMatrix());
}

//***********************************************************************************************
//FUNCTION:
void CRenderer::update()
{
	m_pCamera->update();
}