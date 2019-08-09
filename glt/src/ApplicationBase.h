#pragma once
#include "Common.h"
#include "Renderer.h"
#include "Camera.h"

namespace glt
{
	class CWindow;

	class CApplicationBase
	{
	public:
		CApplicationBase() = default;
		virtual ~CApplicationBase() = default;

		void run();

		void setWindowWidth(int vWidth) { m_WindowInfo.Width = vWidth; }
		void setWindowHeight(int vHeight) { m_WindowInfo.Height = vHeight; }
		void setWindowTitle(const char* vTitle) { m_WindowInfo.Title = vTitle; }

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