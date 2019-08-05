#pragma once
#include "glt/ApplicationBase.h"

class CMyApplication : public glt::CApplicationBase
{
public:
	bool _initV() override;
	void _updateV() override;
	void _destroyV() override;
};