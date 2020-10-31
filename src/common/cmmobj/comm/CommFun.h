#pragma once
#include <string>
#include <list>
#include <map>
#include <vector>
#include <fstream>
#include <iostream>
#include <sys/time.h>
#include <unistd.h>
#include <net/if.h>
#include <iconv.h>
#include "CommDefine.h"
#include "boost/BoostFun.h"

#ifndef SafeDelete
#define SafeDelete( ptr ) \
	do \
	{ \
	if ( (ptr) != NULL ) \
		{ \
		delete (ptr); \
		(ptr) = NULL; \
		} \
	} while ( false )
#endif// SafeDelete

#ifndef SafeDeleteArray
#define SafeDeleteArray( ptr ) \
	do \
	{ \
	if ( (ptr) != NULL ) \
		{ \
		delete[] (ptr); \
		(ptr) = NULL; \
		} \
	} while ( false )
#endif// SafeDeleteArray

static void StrToHex(unsigned char *pbDest, unsigned char *pbSrc, int nLen)
{
	char h1, h2;
	unsigned char s1, s2;
	int i;

	for (i = 0; i < nLen; i++)
	{
		h1 = pbSrc[2 * i];
		h2 = pbSrc[2 * i + 1];

		s1 = (unsigned char)(toupper(h1) - 0x30);
		if (s1 > 9)
			s1 -= 7;

		s2 = (unsigned char)(toupper(h2) - 0x30);
		if (s2 > 9)
			s2 -= 7;

		pbDest[i] = s1 * 16 + s2;
	}
}

static void HexToStr(unsigned char *pbDest, unsigned char *pbSrc, int nLen)
{
	char ddl, ddh;
	int i;

	for (i = 0; i < nLen; i++)
	{
		ddh = 48 + pbSrc[i] / 16;
		ddl = 48 + pbSrc[i] % 16;
		if (ddh > 57) ddh = ddh + 7;
		if (ddl > 57) ddl = ddl + 7;
		pbDest[i * 2] = ddh;
		pbDest[i * 2 + 1] = ddl;
	}

	pbDest[nLen * 2] = '\0';
}

static time_t GetUTCTime()
{
	time_t timep;
	struct tm *p;
	time(&timep);
	p = localtime(&timep);
	timep = mktime(p);
	return timep;
}

static int GetTimezone()
{
	int time_zone = 0;
	time_t time_utc;
	struct tm tm_local;

	// Get the UTC time
	time(&time_utc);

	using namespace boost::date_time;
	// Get the local time
	// Use localtime_r for threads safe
	c_time::localtime(&time_utc, &tm_local);

	time_t time_local;
	struct tm tm_gmt;

	// Change tm to time_t
	time_local = mktime(&tm_local);

	// Change it to GMT tm
	c_time::gmtime(&time_utc, &tm_gmt);

	time_zone = tm_local.tm_hour - tm_gmt.tm_hour;

	if (time_zone < -12) {
		time_zone += 24;
	}
	else if (time_zone > 12) {
		time_zone -= 24;
	}
	return time_zone;
}

static int getlocalinfo(std::string& strIp, std::string& strMac)
{
	char mac[18] = { 0 };
	char ip[32] = { 0 };

	int fd;
	int interfaceNum = 0;
	struct ifreq buf[16];
	struct ifconf ifc;
	struct ifreq ifrcopy;

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("socket");
		close(fd);
		return -1;
	}

	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = (caddr_t)buf;
	if (!ioctl(fd, SIOCGIFCONF, (char *)&ifc))
	{
		interfaceNum = ifc.ifc_len / sizeof(struct ifreq);
		int nIndex = 0;
		while (nIndex++ < interfaceNum)
		{
			//ignore the interface that not up or not runing
			ifrcopy = buf[nIndex];
			if (ioctl(fd, SIOCGIFFLAGS, &ifrcopy))
			{
				close(fd);
				return -1;
			}

			//get the IP of this interface
			if (!ioctl(fd, SIOCGIFADDR, (char *)&buf[nIndex]))
			{
				char* pIpAddr = (char*)inet_ntoa(((struct sockaddr_in *)&(buf[nIndex].ifr_addr))->sin_addr);
				if (strcmp(pIpAddr, "127.0.0.1") == 0)
				{
					continue;
				}

				snprintf(ip, sizeof(ip), "%s", pIpAddr);
				strIp = std::string(ip);
			}
			else
			{
				close(fd);
				return -1;
			}

			//get the mac of this interface
			if (!ioctl(fd, SIOCGIFHWADDR, (char *)(&buf[nIndex])))
			{
				memset(mac, 0, sizeof(mac));
				snprintf(mac, sizeof(mac), "%02x:%02x:%02x:%02x:%02x:%02x",
					(unsigned char)buf[nIndex].ifr_hwaddr.sa_data[0],
					(unsigned char)buf[nIndex].ifr_hwaddr.sa_data[1],
					(unsigned char)buf[nIndex].ifr_hwaddr.sa_data[2],
					(unsigned char)buf[nIndex].ifr_hwaddr.sa_data[3],
					(unsigned char)buf[nIndex].ifr_hwaddr.sa_data[4],
					(unsigned char)buf[nIndex].ifr_hwaddr.sa_data[5]);
				strMac = std::string(mac);
			}
			else
			{
				close(fd);
				return -1;
			}

			return 0;
		}
	}
	else
	{
		close(fd);
		return -1;
	}

	close(fd);

	return 0;
}

/*
  unicode�����
*/
const int nWcbBuffLen = 4 * 1024;
static int EncodeUCS(char *SourceBuf, char *DestBuf)
{
	int len, i, j = 0;
	wchar_t wcbuf[nWcbBuffLen] = { 0 };

	setlocale(LC_ALL, "");
	len = mbstowcs(wcbuf, SourceBuf, nWcbBuffLen); /* convert mutibytes string to wide charater string */
	for (i = 0; i < len; i++)
	{
		DestBuf[j++] = wcbuf[i] >> 8;     /* height byte */
		DestBuf[j++] = wcbuf[i] & 0xff;   /* low byte */
	}
	return len * 2;
}

static int DecodeUCS(unsigned char *SourceBuf, char *DestBuf, int len)
{
	wchar_t wcbuf[nWcbBuffLen] = { 0 };
	setlocale(LC_ALL, "");
	for (int i = 0; i < len / 2; i++)
	{
		wcbuf[i] = SourceBuf[2 * i];    // height byte
		wcbuf[i] = (wcbuf[i] << 8) + SourceBuf[2 * i + 1];    // low byte
	}

	int size = wcstombs(DestBuf, wcbuf, 2 * len); /* convert wide charater string to mutibytes string */
	DestBuf[size] = '\0';
	return size;
}


static int Iconv(iconv_t &cd, char* sourcebuf, int sourcelen, char* destbuf, int destlen)
{
	char **source = &sourcebuf;
	char **dest = &destbuf;
	int len = iconv(cd, source, (size_t*)&sourcelen, dest, (size_t*)&destlen);
	if (-1 == len)
	{
		return -1;
	}
	iconv_close(cd);
	destbuf[len] = '\0';
	return len;
}

//UTF-8��GB2312��ת��
static int U2G(char* utf8, int utfLen, char* gb2312, int gbLen)
{
	iconv_t cd;
	if (0 == (cd = iconv_open("gb2312", "utf-8")))
	{
		return -1;
	}
	return Iconv(cd, utf8, utfLen, gb2312, gbLen);
}

//GB2312��UTF-8��ת��
static int G2U(char* gb2312, int gbLen, char* utf8, int utfLen)
{
	iconv_t cd;
	if (0 == (cd = iconv_open("utf-8", "gb2312")))
	{
		return -1;
	}
	return Iconv(cd, gb2312, gbLen, utf8, utfLen);
}

static bool Strtok(const std::string& strSrc, const char* split, std::vector<std::string>& lstStr)
{
	int nLen = strSrc.length();
	if (0 >= nLen)
	{
		return false;
	}

	char* pStr = new char[nLen + 1];
	memset(pStr, 0, nLen + 1);
	memcpy(pStr, (char*)strSrc.c_str(), nLen);

	char * p;
	p = strtok(pStr, split);
	while (p != NULL)
	{
		lstStr.push_back(p);
		p = strtok(NULL, split);
	}

	SafeDeleteArray(pStr);

	int nListSize = lstStr.size();
	if (0 >= nListSize)
	{
		return false;
	}
	return true;
}

//IP to 32 bit int
static unsigned int IPStrToInt(const char *ip)
{
	unsigned uResult = 0;
	int nShift = 24;
	int temp = 0;
	const char *pStart = ip;
	const char *pEnd = ip;

	while (*pEnd != '\0')
	{
		while (*pEnd != '.' && *pEnd != '\0')
		{
			pEnd++;
		}
		temp = 0;
		for (pStart; pStart != pEnd; ++pStart)
		{
			temp = temp * 10 + *pStart - '0';
		}

		uResult += temp << nShift;
		nShift -= 8;

		if (*pEnd == '\0')
			break;
		pStart = pEnd + 1;
		pEnd++;
	}

	return uResult;
}

static char *IntIpToStr(const int ip, char *buf)
{
	sprintf(buf, "%u.%u.%u.%u",
		(unsigned char)*((char *)&ip + 0),
		(unsigned char)*((char *)&ip + 1),
		(unsigned char)*((char *)&ip + 2),
		(unsigned char)*((char *)&ip + 3));
	return buf;
}

//
static std::string GetSecondTime()
{
	struct timeval tv;
	struct tm *     time_ptr;
	memset(&tv, 0, sizeof(timeval));
	gettimeofday(&tv, NULL);
	time_ptr = localtime(&tv.tv_sec);
	int millonsecond = (int)tv.tv_usec / 1000;
	char szTime[50];
	int s32TimeZone = GetTimezone();
	s32TimeZone = (0 >= s32TimeZone) ? 8 : s32TimeZone;
	sprintf(szTime, "%d-%02d-%02d %02d:%02d:%02d",
		time_ptr->tm_year + 1900,
		time_ptr->tm_mon + 1,
		time_ptr->tm_mday,
		time_ptr->tm_hour + s32TimeZone,
		time_ptr->tm_min,
		time_ptr->tm_sec);
	std::string strTime(szTime);
	return strTime;
}

//获取当前时间(毫秒)
static std::string GetMillionTime()
{
	struct timeval tv;
	struct tm *     time_ptr;
	memset(&tv, 0, sizeof(timeval));
	gettimeofday(&tv, NULL);
	time_ptr = localtime(&tv.tv_sec);
	int millonsecond = (int)tv.tv_usec / 1000;
	char szTime[50];

	int s32TimeZone = GetTimezone();
	s32TimeZone = (0 >= s32TimeZone) ? 8 : s32TimeZone;
	sprintf(szTime, "%d-%02d-%02d %02d:%02d:%02d.%.03d",
		time_ptr->tm_year + 1900,
		time_ptr->tm_mon + 1,
		time_ptr->tm_mday,
		time_ptr->tm_hour + s32TimeZone,
		time_ptr->tm_min,
		time_ptr->tm_sec,
		millonsecond);
	std::string strTime(szTime);
	return strTime;
}

//获取当前时钟(时：分：秒)
static std::string GetCurClock()
{
	struct timeval tv;
	struct tm *     time_ptr;
	memset(&tv, 0, sizeof(timeval));
	gettimeofday(&tv, NULL);
	time_ptr = localtime(&tv.tv_sec);
	char szTime[50];

	sprintf(szTime, "%02d:%02d:%02d",
		time_ptr->tm_hour,
		time_ptr->tm_min,
		time_ptr->tm_sec);
	return std::string(szTime);
}

//检测超时
static bool IsTimeOut(time_t InitTime, int nTimeOut)
{
	time_t CurTime = time(NULL);
	if (nTimeOut < (CurTime - InitTime))
	{
		return true;
	}
	return false;
}

//String to Binary
static void StringToBinary(const std::string& strSrc, std::string& strDes)
{
	if (strSrc.empty()) {
		return;
	}

	for (int i = 0; i < strSrc.size(); ++i)
	{
		char c = strSrc.at(i);
		unsigned char k = 0x80;
		for (int i = 0; i < 8; i++, k >>= 1) {
			if (c & k) {
				strDes.append("1");
			}
			else {
				strDes.append("0");
			}
		}
	}
}

//退出站点
static void ExitAAS(int code)
{
	_exit(code);
}

//读图片文件数据
static std::string ReadFileData(const std::string& strPath)
{
	//从文件中读取
	ifstream fin(strPath.c_str(), ios::binary);

	//创建字符串流对象
	ostringstream sin;

	//把文件流中的字符输入到字符串流中
	sin << fin.rdbuf();

	//获取字符串流中的字符串
	std::string strData = sin.str();

	//关闭和清除文件流对象
	fin.close();
	fin.clear();

	return strData;
}