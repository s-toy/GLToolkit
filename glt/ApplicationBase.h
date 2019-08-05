#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Common.h"

namespace glt
{
	class CWindow;
	class CCamera;

	class CApplicationBase
	{
	public:
		CApplicationBase();
		virtual ~CApplicationBase();

		void run();

		const CCamera* getCamera() const { return m_pCamera; }

	protected:
		_DISALLOW_COPY_AND_ASSIGN(CApplicationBase);

		virtual bool _initV();
		virtual void _updateV();
		virtual void _destroyV();

	private:
		CWindow* m_pWindow = nullptr;
		SWindowInfo m_WindowInfo = {};

		CCamera* m_pCamera = nullptr;
	};
}