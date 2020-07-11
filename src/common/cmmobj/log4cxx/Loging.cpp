#include "Loging.h"
#include <iostream>
#include <log4cxx/logger.h>
#include <log4cxx/propertyconfigurator.h>

#define RELOAD_CONFIG 10000

using namespace std;
using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace common_cmmobj;

CLoging::CLoging()
{
}

CLoging::CLoging(const CLoging & logObj)
{
	if (this != &logObj)
	{
		m_s32Level_ = logObj.m_s32Level_;
		m_strLogName_ = logObj.m_strLogName_;
		m_strFileName_ = logObj.m_strFileName_;
		m_s32Line_ = logObj.m_s32Line_;
	}
}

CLoging::~CLoging()
{
}

CLoging & CLoging::stream(int s32Level, const std::string & strLogName, const std::string & strFileName, const int s32Line)
{
	static bool bInit = false;
	if (!bInit)
	{
		log4cxx::PropertyConfigurator::configureAndWatch("./log.properties", RELOAD_CONFIG);
		bInit = true;
	}
	m_s32Level_ = s32Level;
	m_strLogName_ = strLogName;
	m_strFileName_ = strFileName;
	m_s32Line_ = s32Line;
	return *this;
}

CLoging& CLoging::operator<<(const char* pData)
{
	if (!pData)
	{
		return *this;
	}

	try
	{
		stringstream strM;
		strM << "[" << m_strFileName_ << ":" << m_s32Line_ << "] " << pData;
		log4cxx::LoggerPtr logger = log4cxx::Logger::getLogger(m_strLogName_);

		switch (m_s32Level_)
		{
		case DEBUG:
		{
			LOG4CXX_DEBUG(logger, strM.str());
		}
		break;
		case INFOR:
		{
			LOG4CXX_INFO(logger, strM.str());
		}
		break;
		case WARN:
		{
			LOG4CXX_WARN(logger, strM.str());
		}
		break;
		case ERRORR:
		{
			LOG4CXX_ERROR(logger, strM.str());
		}
		break;
		case FATAL:
		{
			LOG4CXX_FATAL(logger, strM.str());
		}
		break;
		default:
			break;
		}
	}
	catch (std::exception& e)
	{
		cout << "log4cxx::Logger::getLogger fail: " << e.what() << endl;
	}
	catch (...)
	{
		cout << "log4cxx::Logger::getLogger fail: " << endl;
	}

	return *this;
}

CLoging& CLoging::operator<<(const string& strData)
{
	*this << strData.c_str();
	return *this;
}

CLoging & CLoging::operator=(const CLoging & logObj)
{
	if (this == &logObj)
	{
		return *this;
	}

	m_s32Level_ = logObj.m_s32Level_;
	m_strLogName_ = logObj.m_strLogName_;
	m_strFileName_ = logObj.m_strFileName_;
	m_s32Line_ = logObj.m_s32Line_;

	return *this;
}