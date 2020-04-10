#pragma once
#pragma warning (disable: 4251)

#include "Export.h"
#include <chrono>

namespace glt
{
	class GLT_DECLSPEC CCPUTimer
	{
	public:
		using TimePoint = std::chrono::steady_clock::time_point;

		enum class ETimerStatus : unsigned char
		{
			Running = 0,
			Stopped,
		};

		CCPUTimer() = default;
		~CCPUTimer() = default;

		bool start();
		void stop();

		double getElapsedTime();
		double getElapsedTimeInMS();
		double getTimestamp();

		ETimerStatus getStatus() const { return m_Status; }

	private:
		bool m_IsElapsedTimeReturned = false;  //这个变量的目的在于，停止计数器后，只能调用一次getElapsedTimme*()返回时间。如果不重新启动计时器，多次
		                                       //调用getElaspedTime*()将会返回0（第一次除外）。这样做的目的在于防止用户忘了重新启动计时器而又多次调用
		                                       //getElaspedTime*()而得到不正确的时间（相对于不正确的时间来说，0更容易被发现是getElapsedTime*()调用错误
		                                       //注意：如上机制没有考虑多线程情况。如果在多线程下，停止计数器后，不同线程同时调用getElapsedTime*()，不
		                                       //保证只有一个线程获得正确时间，而其他线程都获得0
		TimePoint m_BeginTime;
		TimePoint m_EndTime;
		ETimerStatus m_Status = ETimerStatus::Stopped;
	};
}