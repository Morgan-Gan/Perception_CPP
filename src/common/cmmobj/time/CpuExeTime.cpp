#include "CpuExeTime.h"
#include "CommonFun.h"

using namespace std;
using namespace common_cmmobj;
using namespace common_template;

const int s32DefaultTimeout = 100;

CCpuExeTime::CCpuExeTime(int s32TimeOut, const string& strExeName,const CLoging& logObj):
	m_StartTime(0),
	m_EndTime(0),
	m_s32TimeOut(s32TimeOut),
	m_logObj(logObj),
	m_strExeName(strExeName),
	m_bPrintInterval(false)
{
	m_StartTime = GetCurrentTime();
}

CCpuExeTime::CCpuExeTime(const string & strExeName, const CLoging & logObj):
	m_StartTime(0),
	m_EndTime(0),
	m_s32TimeOut(s32DefaultTimeout),
	m_logObj(logObj),
	m_strExeName(strExeName),
	m_bPrintInterval(true)
{
	m_StartTime = GetCurrentTime();
}

CCpuExeTime::~CCpuExeTime()
{
	m_EndTime = GetCurrentTime();

	int s32Interval = (m_EndTime - m_StartTime) / 1000;
	if (s32Interval >= m_s32TimeOut || m_bPrintInterval)
	{
		m_logObj << string_format("%s|Processing timeout(%d)!", m_strExeName.c_str(),s32Interval);
	}
}

long CCpuExeTime::GetCurrentTime()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);    
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}