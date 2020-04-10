#include "CpuTimer.h"

using namespace glt;

//********************************************************************************
//FUNCTION:
bool CCPUTimer::start()
{
	if (m_Status == ETimerStatus::Running) return false;

	m_Status = ETimerStatus::Running;
	m_IsElapsedTimeReturned = false;
	m_BeginTime = std::chrono::steady_clock::now();
	return true;
}

//********************************************************************************
//FUNCTION:
void CCPUTimer::stop()
{
	if (m_Status != ETimerStatus::Running) return;

	m_EndTime = std::chrono::steady_clock::now();
	m_Status = ETimerStatus::Stopped;
}

//********************************************************************************
//FUNCTION:
double CCPUTimer::getElapsedTime()
{
	if ((m_Status != ETimerStatus::Stopped) || m_IsElapsedTimeReturned) return 0;
	double t = std::chrono::duration<double>(m_EndTime - m_BeginTime).count();
	m_IsElapsedTimeReturned = true;
	return t;
}

//********************************************************************************
//FUNCTION:
double CCPUTimer::getElapsedTimeInMS()
{
	if ((m_Status != ETimerStatus::Stopped) || m_IsElapsedTimeReturned) return 0;
	double t= std::chrono::duration<double, std::ratio<1, 1000>>(m_EndTime - m_BeginTime).count();
	m_IsElapsedTimeReturned = true;
	return t;
}

//*****************************************************************************************
//FUNCTION:
double CCPUTimer::getTimestamp()
{
	_ASSERTE(m_Status == CCPUTimer::ETimerStatus::Running);
	return std::chrono::duration<double, std::ratio<1, 1000>>(std::chrono::steady_clock::now() - m_BeginTime).count();
}