#include "ApplicationBase.h"
#include "ShaderProgram.h"
#include "Model.h"

using namespace glt;

class CMyApplication : public CApplicationBase
{
public:
	bool _initV() override
	{
		m_pShaderProgram = std::make_unique<CShaderProgram>();
		m_pShaderProgram->addShader("shaders/skeletal_animation.vert", EShaderType::VERTEX_SHADER);
		m_pShaderProgram->addShader("shaders/skeletal_animation.frag", EShaderType::FRAGMENT_SHADER);

		m_pModel = std::make_unique<CModel>("../../resource/models/sphere-bot/Armature_001-(COLLADA_3 (COLLAborative Design Activity)).dae");
		m_pModel->setRotation(1.57, glm::vec3(1.0f, 0.0f, 0.0f));

		CRenderer::getInstance()->fetchCamera()->setPosition(glm::dvec3(0, 1.5, 6));

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
	if (!App.init(SWindowInfo(1024, 576, "Skeletal Animation Demo"))) return -1;
	App.run();

	return 0;
}