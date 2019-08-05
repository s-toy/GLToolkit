#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Common.h"

namespace glt
{
	class CWindow;
	class CCamera;

	class CApplicationBase
	{
	public:
		CApplicationBase() = default;
		virtual ~CApplicationBase() = default;

		void run();

		void setWindowWidth(int vWidth) { m_WindowInfo.Width = vWidth; }
		void setWindowHeight(int vHeight) { m_WindowInfo.Height = vHeight; }

		CCamera* fetchCamera() const { return _pCamera; }

	protected:
		_DISALLOW_COPY_AND_ASSIGN(CApplicationBase);

		virtual bool _initV() { return true; }
		virtual void _updateV() {}
		virtual void _destroyV() {}

	protected:
		bool __init();
		void __destroy();

		CWindow* _pWindow = nullptr;
		CCamera* _pCamera = nullptr;

	private:
		SWindowInfo m_WindowInfo = {};
	};
}