#include "ApplicationBase.h"
#include "ShaderProgram.h"
#include "Model.h"

using namespace glt;

class CMyApplication : public CApplicationBase
{
public:
	bool _initV() override
	{
		setDisplayStatusHint();

		m_pShaderProgram = std::make_unique<CShaderProgram>();
		m_pShaderProgram->addShader("shaders/perpixel_shading_vs.glsl", EShaderType::VERTEX_SHADER);
		m_pShaderProgram->addShader("shaders/perpixel_shading_fs.glsl", EShaderType::FRAGMENT_SHADER);

		m_pModel = std::make_unique<CModel>("../../resource/models/nanosuit/nanosuit.obj");
		m_pModel->setPosition(glm::vec3(0.0f, -1.5f, 0.0f));
		m_pModel->setScale(glm::vec3(0.2f, 0.2f, 0.2f));

		CRenderer::getInstance()->fetchCamera()->setPosition(glm::dvec3(0, 0, 5));

		return true;
	}

	void _renderV() override
	{
		CRenderer::getInstance()->clear();
		CRenderer::getInstance()->draw(*m_pModel, *m_pShaderProgram);
	}

private:
	std::unique_ptr<CShaderProgram> m_pShaderProgram = nullptr;
	std::unique_ptr<CModel> m_pModel = nullptr;
};

int main()
{
	CMyApplication App;
	if (!App.init(SWindowInfo(1024, 576, "Order Independent Transparency"))) return -1;
	App.run();

	return 0;
}