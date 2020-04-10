#include "ApplicationBase.h"
#include "ShaderProgram.h"
#include "Model.h"
#include "MonitorManager.h"
#include "InputManager.h"

using namespace glt;

const glm::vec2 g_WindowSize = { 1024, 576 };

class CMyApplication : public CApplicationBase
{
protected:
	bool _initV() override
	{
		setDisplayStatusHint();

		m_pShaderProgram = std::make_unique<CShaderProgram>();
		m_pShaderProgram->addShader("shaders/draw_screen_quad_vs.glsl", EShaderType::VERTEX_SHADER);
		m_pShaderProgram->addShader("shaders/shadertoy_fs.glsl", EShaderType::FRAGMENT_SHADER);

		return true;
	}

	void _renderV() override
	{
		CRenderer::getInstance()->clear();
		CRenderer::getInstance()->drawScreenQuad(*m_pShaderProgram);
	}

	void _updateV() override
	{
		m_pShaderProgram->bind();

		m_pShaderProgram->updateUniform2f("iResolution", g_WindowSize);
		m_pShaderProgram->updateUniform1f("iTime", static_cast<float>(getTime()));

		auto CursorPos = CInputManager::getInstance()->getCursorPos();
		auto MouseStatus = CInputManager::getInstance()->getMouseButtonStatus();
		float ZValue = MouseStatus[0] ? 1.0 : -1.0;
		m_pShaderProgram->updateUniform4f("iMouse", glm::vec4(CursorPos[0], CursorPos[1], ZValue, 0));

		m_pShaderProgram->unbind();
	}

private:
	std::unique_ptr<CShaderProgram> m_pShaderProgram = nullptr;
};

int main()
{
	CMyApplication App;
	if (!App.init(SWindowInfo(g_WindowSize.x, g_WindowSize.y, "Shadertoy Demo"))) return -1;
	App.run();

	return 0;
}