#pragma once
#include <functional>
#include "comm/Endian.h"
#include "comm/CommFun.h"
#include "comm/Convt.h"
#include "comm/CalcCrc.h"
#include "boost/BoostFun.h"
#include "buf/SmartBuffer.h"
#include "mutex/LibMutex.h"
#include "msgbus/MessageBus.h"

namespace common_cmmobj
{
	#pragma pack(push, 1)
		typedef struct 
		{
			char flag;				    //包头标识，暂定为0x7D
			int  length;		        //数据包总长度,指明包(包头+包体+包尾)总长度，无包体时即包头的长度
			unsigned char version;		//第一版本为01
			short seqno;				//序号
			unsigned char cmd;			//消息码
			unsigned char type;         //表请求或应答
			char result;				//返回结果：< 0 错误，>= 0 成功
		} SPkgHead;

		//包尾定义
		typedef struct 
		{
			short crc;  				//包括长度、序号、消息码、结果码、包体的校验和
			char flag;  				//用于检测数据包是否发送结束，暂定为0x0B
		}SPkgTail;
	#pragma pack(pop)
	enum { MSG_REQ = 1, MSG_RSP = 2 };
	
	class CNetTcpDataPkg
	{
		using CMutexType = common_cmmobj::CMutex;
		using FuncType = std::function<void()>;
		using FuncPrintType = std::function<void(unsigned char, const char*, int)>;
		using FuncDisType = std::function<void(unsigned int,int,const char*, int, const std::string&,const std::string&)>;

	public:
		CNetTcpDataPkg() = default;
		~CNetTcpDataPkg() = default;
		
		template<class FuncA,class FuncB>
		CNetTcpDataPkg(const std::string& strServerFlag,const std::string& strCliFlag,FuncA f,FuncB g):
		m_bHaveHead(false),
		m_seqno(0),
		m_nOriginalDataLen(0),
		m_u8Version(0x04),
		m_DistributePkgDataFunc(f),
		m_PrintPkgDataFunc(g),
		m_strServerFlag(strServerFlag),
		m_strCliFlag(strCliFlag)
		{

		}

	public:
		//tcp网络数据解包
		bool UnDataPkg(const char *recvbuf, unsigned int recvlen);

		//tcp网络数据封包
		bool PkgProtocolData(int nSendType, unsigned short command, const char* input, unsigned int inputlen, char* output, unsigned int& outputlen, const unsigned int outsize);
	
	private:
		//tcp数据分包和封包相关函数
		void SetHaveHead(bool bRecvHead);
		bool GetHaveHead();
		void ParsePkgData();
		bool ReadPkgHead();
		void ReadWholePkg();
		void MakeSendPkgHead(SPkgHead& tPkgHead);
		void MakeResponePkgHead(SPkgHead& tPkgHead);
		char* AllocateMem(const int nBodyBufferLen, int& nPkgLen);
		unsigned char GetProtocolVersion();
		void SetProtocolVersion(unsigned char);

	private:
		//数据缓存
		common_cmmobj::CSmartBuffer m_receive_buffer;
		common_cmmobj::CSmartBuffer m_pkg_buffer;
		bool m_bHaveHead;
		short m_seqno;

		//回调函数
		FuncPrintType m_PrintPkgDataFunc;
		FuncDisType m_DistributePkgDataFunc;

		//协议版本号
		unsigned char m_u8Version;

		//没有反转码的原始数据包指针和长度
		int m_nOriginalDataLen;

		//数据源
		std::string m_strServerFlag;
		std::string m_strCliFlag;
	};
}
