#pragma once
#include <vector>
#include <GLFW/glfw3.h>
#include "Common.h"

namespace glt
{
	struct SMonitorInfo
	{
		SVector2		Resolution = {};
		SRect			DisplayArea = {};
		GLFWmonitor*	pMonitor = nullptr;
	};

	class CMonitorManager
	{
	public:
		virtual ~CMonitorManager();
		_SINGLETON(CMonitorManager);

		const SMonitorInfo& getPrimaryMonitorInfo() const;
		const SMonitorInfo& getMonitorInfo(uint32_t vMonitorID) const;
		size_t getNumMonitors() const { return m_MonitorInfoSet.size(); }

	private:
		CMonitorManager();
		_DISALLOW_COPY_AND_ASSIGN(CMonitorManager);

		void __queryMonitorsInfo();

		std::vector<SMonitorInfo> m_MonitorInfoSet;
	};
}