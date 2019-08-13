#pragma once
#include "Common.h"
#include "Renderer.h"

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

	protected:
		_DISALLOW_COPY_AND_ASSIGN(CApplicationBase);

		virtual bool _initV() { return true; }
		virtual void _updateV() {}
		virtual void _destroyV() {}

	protected:
		bool __init();
		bool __initIMGUI();
		void __destroy();

		CWindow* _pWindow = nullptr;

	private:
		SWindowInfo m_WindowInfo = {};
	};
}