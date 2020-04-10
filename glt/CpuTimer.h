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
		bool m_IsElapsedTimeReturned = false;  //���������Ŀ�����ڣ�ֹͣ��������ֻ�ܵ���һ��getElapsedTimme*()����ʱ�䡣���������������ʱ�������
		                                       //����getElaspedTime*()���᷵��0����һ�γ��⣩����������Ŀ�����ڷ�ֹ�û���������������ʱ�����ֶ�ε���
		                                       //getElaspedTime*()���õ�����ȷ��ʱ�䣨����ڲ���ȷ��ʱ����˵��0�����ױ�������getElapsedTime*()���ô���
		                                       //ע�⣺���ϻ���û�п��Ƕ��߳����������ڶ��߳��£�ֹͣ�������󣬲�ͬ�߳�ͬʱ����getElapsedTime*()����
		                                       //��ֻ֤��һ���̻߳����ȷʱ�䣬�������̶߳����0
		TimePoint m_BeginTime;
		TimePoint m_EndTime;
		ETimerStatus m_Status = ETimerStatus::Stopped;
	};
}