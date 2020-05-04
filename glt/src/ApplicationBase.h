#pragma once
#include <imgui/imgui.h>
#include "Common.h"
#include "Renderer.h"
#include "CpuTimer.h"
#include "Export.h"

namespace glt
{
	class CWindow;

	class GLT_DECLSPEC CApplicationBase
	{
	public:
		CApplicationBase() = default;
		virtual ~CApplicationBase() = default;

		bool init(const SWindowInfo& vWindowInfo = {});
		void run();

		void setDisplayStatusHint() { m_DisplayAppStatus = true; }

		double getTime() { return m_CPUTimer.getTimestamp() / 1000.0; }

	protected:
		_DISALLOW_COPY_AND_ASSIGN(CApplicationBase);

		virtual bool _initV() { return true; }
		virtual void _updateV() {}
		virtual void _renderV() {}
		virtual void _onGuiV() {}

	protected:
		bool __initIMGUI();
		void __renderGUI();
		void __destroy();

		CWindow* _pWindow = nullptr;

	private:
		SWindowInfo m_WindowInfo = {};

		bool m_IsInitialized = false;
		bool m_DisplayAppStatus = false;

		CCPUTimer m_CPUTimer;
	};
}