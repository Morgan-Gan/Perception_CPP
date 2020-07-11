#pragma once
#include <stdio.h>
#ifndef _WIN32
#include <sys/time.h>
#else
#include <time.h>
#endif
#include <string>
#include "log4cxx/Loging.h"

namespace common_cmmobj
{
	class CCpuExeTime
	{
	public:
		CCpuExeTime(int s32TimeOut, const std::string& strExeName, const CLoging& logObj);
		CCpuExeTime(const std::string& strExeName, const CLoging& logObj);
		~CCpuExeTime();

	protected:
		long GetCurrentTime();

	private:
		long m_StartTime;
		long m_EndTime;

		int m_s32TimeOut;			//��λΪ����
		CLoging m_logObj;
		std::string m_strExeName;
		bool m_bPrintInterval;
	};
}