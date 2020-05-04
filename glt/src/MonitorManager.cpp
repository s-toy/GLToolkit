#include "MonitorManager.h"

using namespace glt;

//TODO: Ŀǰ��ʵ�ּ���������һ��monitor, ��Ҫ����û��monitor�����

//********************************************************************
//FUNCTION:
CMonitorManager::CMonitorManager()
{
	glfwInit();
	__queryMonitorsInfo();
}

//********************************************************************
//FUNCTION:
CMonitorManager::~CMonitorManager()
{
}

//*************************************************************
//FUNCTION:
const SMonitorInfo& CMonitorManager::getMonitorInfo(uint32_t vMonitorID) const
{
	_ASSERTE(vMonitorID < m_MonitorInfoSet.size());
	return m_MonitorInfoSet[vMonitorID];
}

//*************************************************************
//FUNCTION:
const SMonitorInfo& CMonitorManager::getPrimaryMonitorInfo() const
{
	_ASSERTE(m_MonitorInfoSet.size() > 0);
	return m_MonitorInfoSet[0];	//NOTE: �����е�0��λ��һ��������ʾ��
}

//*************************************************************
//FUNCTION:
void CMonitorManager::__queryMonitorsInfo()
{
	int MonitorsCount;
	GLFWmonitor** ppMonitors = glfwGetMonitors(&MonitorsCount);
	_ASSERTE(ppMonitors && MonitorsCount > 0);

	for (auto i = 0; i < MonitorsCount; ++i)
	{
		GLFWmonitor* pMonitor = ppMonitors[i];
		_ASSERTE(pMonitor);

		const GLFWvidmode* pMode = glfwGetVideoMode(pMonitor);
		_ASSERTE(pMode);

		SRect DisplayArea;
		glfwGetMonitorWorkarea(pMonitor, &DisplayArea.x, &DisplayArea.y, &DisplayArea.w, &DisplayArea.h);

		SMonitorInfo MonitorInfo;
		MonitorInfo.Resolution = { pMode->width, pMode->height };
		MonitorInfo.pMonitor = pMonitor;
		MonitorInfo.DisplayArea = DisplayArea;
		m_MonitorInfoSet.emplace_back(MonitorInfo);
	}
}