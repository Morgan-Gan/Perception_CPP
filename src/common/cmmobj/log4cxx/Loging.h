#pragma once
#include <string>

namespace common_cmmobj
{

#define DEBUG  (0)
#define INFOR  (1)
#define WARN   (2)
#define ERRORR (3)
#define FATAL  (4)

	class CLoging final
	{
	public:
		CLoging();
		CLoging(const CLoging& logObj);
		~CLoging();

		CLoging& stream(int s32Level, const std::string& strLogName, const std::string& strFileName, const int s32Line);
		CLoging& operator<<(const char* pData);
		CLoging& operator<<(const std::string& strData);
		CLoging& operator=(const CLoging& logObj);

	private:
		std::string m_strLogName_;
		std::string m_strFileName_;
		int m_s32Level_;
		int m_s32Line_;
	};

#define LOG_DEBUG(strLogName) CLoging().stream(DEBUG,strLogName,__FILE__,__LINE__)
#define LOG_INFO(strLogName) CLoging().stream(INFOR,strLogName,__FILE__,__LINE__)
#define LOG_WARN(strLogName) CLoging().stream(WARN,strLogName,__FILE__,__LINE__)
#define LOG_ERROR(strLogName) CLoging().stream(ERRORR,strLogName,__FILE__,__LINE__)
#define LOG_FATAL(strLogName) CLoging().stream(FATAL,strLogName,__FILE__,__LINE__)
}