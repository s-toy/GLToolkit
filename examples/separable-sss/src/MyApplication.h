#pragma once
#include "ApplicationBase.h"

namespace glt
{
	class CShaderProgram;
	class CModel;
}

class CMyApplication : public glt::CApplicationBase
{
public:
	bool _initV() override;
	void _updateV() override;
	void _destroyV() override;

private:
	glt::CShaderProgram* m_pShaderProgram = nullptr;
	glt::CModel* m_pModel = nullptr;
};