#include "NetTcpDataPkg.h"
#include "log4cxx/Loging.h"
#include "comm/FuncWrapper.h"
#include <sys/time.h>
#include <unistd.h>
#include <net/if.h>
#include <iconv.h>

#define CURRENT_PROTOCOL_VERSION   (0x04)
using namespace std;
using namespace common_cmmobj;
using namespace common_template;

const int nPkgHeadLen = sizeof(SPkgHead);
const int nPkgTailLen = sizeof(SPkgTail);
const int nPkgNobodyLen = nPkgHeadLen + nPkgTailLen;
const int nHeadFlagLen = 1;
const int nTailFlagLen = 1;
const int s32HeardOffSet = nHeadFlagLen + nIntLen;
char cHeadFlag = 0x7D;
char cTailFlag = 0x0B;


bool CNetTcpDataPkg::UnDataPkg(const char *recvbuf, unsigned int recvlen)
{
	if (0 == recvlen || nMaxPkgLen < recvlen) 
	{
		return false;
	}

	//clear up super len buffer data 
	if (m_receive_buffer.length() > nMaxRecCache)
	{
		m_receive_buffer.empty();
	}
	m_receive_buffer.append(recvbuf, recvlen);

	//截包
	if (!GetHaveHead())
	{
		//to deal with the package body data only after receiving the entire header data 
		if (ReadPkgHead())
		{
			ReadWholePkg();
		}
	}
	else
	{
		ReadWholePkg();
	}

	return true;
}

bool CNetTcpDataPkg::ReadPkgHead()
{
	//find 0x7D heard flag
	int nHeadIndex = m_receive_buffer.find(cHeadFlag);
	if (-1 == nHeadIndex)
	{
		m_receive_buffer.empty();
		SetHaveHead(false);
		return false;
	}
	else if (0 < nHeadIndex)
	{
		//delete dirty data befor 7d
		m_receive_buffer.truncate(nHeadIndex);	
	}

	SetHaveHead(true);
	return true;
}

//Receive package body base on own heard
void CNetTcpDataPkg::ReadWholePkg()
{
	while (true)
	{
		//May be receive an integer package + part of a package
		if (!GetHaveHead())
		{
			if (!ReadPkgHead())
			{
				break;
			}
		}

		//find tail flag
		int&& nTailIndex = m_receive_buffer.find(cTailFlag);
		if (-1 == nTailIndex)	//continue receive data when no find tail flag 
		{
			return;
		}
		
		//the len of raw package befor inversion code
		m_nOriginalDataLen = nTailIndex + 1;

		//current package len < heard + tail
		if (m_nOriginalDataLen < nPkgNobodyLen) 
		{
			m_receive_buffer.truncate(m_nOriginalDataLen);
			SetHaveHead(false);
			continue;
		}
		
		//inversion code
		char UnCode[nMaxPkgLen] = { 0 };
		int UnConvtLen = UnconvtCode((unsigned char*)(m_receive_buffer.data() + s32HeardOffSet), m_nOriginalDataLen - s32HeardOffSet - nTailFlagLen, (unsigned char*)(UnCode));
		if (0 >= UnConvtLen)
		{
			m_receive_buffer.truncate(m_nOriginalDataLen);
			SetHaveHead(false);
			continue;
		}

		//copy unconvt package data to buffer
		int&& s32RawProtoclDataLen = s32HeardOffSet + UnConvtLen + nTailFlagLen;
		m_pkg_buffer.empty();
		m_pkg_buffer.append(cHeadFlag);
		m_pkg_buffer.append((char*)&s32RawProtoclDataLen,nIntLen);
		m_pkg_buffer.append(UnCode, UnConvtLen);
		m_pkg_buffer.append(cTailFlag);

		//crc check
		SPkgHead* pSPkgHead = (SPkgHead*)m_pkg_buffer.data();
		if(s32RawProtoclDataLen >= nPkgNobodyLen)      
		{
			//包中的crc
			SPkgTail* pTail = (SPkgTail*)(m_pkg_buffer.data() + s32RawProtoclDataLen - nPkgTailLen); //the pointer of tail
			unsigned short&& u16PkgCRC = ntohs(pTail->crc);

			//计算的crc
			unsigned short&& u16CalcCRC = CalcCRC1021((unsigned char*)(m_pkg_buffer.data() + s32HeardOffSet), s32RawProtoclDataLen - s32HeardOffSet - nPkgTailLen);
			
			//比较crc
			if (u16CalcCRC== u16PkgCRC)
			{
				ParsePkgData();	 
			}
		}

		m_receive_buffer.truncate(m_nOriginalDataLen);
		SetHaveHead(false);
	}
}

void CNetTcpDataPkg::SetHaveHead(bool bRecvHead)
{
	m_bHaveHead = bRecvHead;
}

bool CNetTcpDataPkg::GetHaveHead()
{
	return m_bHaveHead;
}

void CNetTcpDataPkg::ParsePkgData()
{
	SPkgHead* pPkgHead = (SPkgHead*)m_pkg_buffer.data();
	
	//设置协议版本
	SetProtocolVersion(pPkgHead->version);
	
	//protocol version judge
	if(CURRENT_PROTOCOL_VERSION >= GetProtocolVersion())
	{
		//update lastest package serial num
		m_seqno = SwapWord(pPkgHead->seqno);

		//transform the whole binary data of inversion code package to string and print
		int nPkgTotalLen = pPkgHead->length;
		if(m_PrintPkgDataFunc)
		{
			FuncWrapper(move(m_PrintPkgDataFunc),move(pPkgHead->cmd),move(m_pkg_buffer.data()),move(nPkgTotalLen));
		}

		//callback the body data of after inversion code and the whole data of no inversion code
		if(m_DistributePkgDataFunc)
		{
			int&& nPkgBodyLen = nPkgTotalLen - nPkgNobodyLen;
			char*&& pPkgBody = (char*)(m_pkg_buffer.data() + nPkgHeadLen);
			FuncWrapper(move(m_DistributePkgDataFunc),move(pPkgHead->cmd),move(pPkgHead->result),move(pPkgBody),move(nPkgBodyLen),m_strServerFlag,m_strCliFlag);
		}
	}
	else
	{
		LOG_WARN("system")<<string_format("Rec cmd(0x%02x) and data version(0x%02x) error!!)",(int)pPkgHead->cmd,(int)pPkgHead->version).c_str();
	}

	//clean up package buffer
	m_pkg_buffer.empty();
}

void CNetTcpDataPkg::MakeSendPkgHead(SPkgHead& tPkgHead)
{
	tPkgHead.type = 0x00;
	tPkgHead.result = 0x00;
}

void CNetTcpDataPkg::MakeResponePkgHead(SPkgHead& tPkgHead)
{
	tPkgHead.type = 0x01;
	tPkgHead.result = 0x01;
    tPkgHead.seqno = SwapWord(m_seqno);
}

bool CNetTcpDataPkg:: PkgProtocolData(int nSendType,unsigned short cmd, const char* input, unsigned int inputlen, char* output, unsigned int& outputlen,const unsigned int outsize)
{
	//Distribution memory space
	int nAllocMemLen = 0;
	char* pBuffer = AllocateMem(inputlen, nAllocMemLen);
	if (NULL == pBuffer || nAllocMemLen > outsize)
	{
		return false;
	}

	//build package heard
	SPkgHead tSPkgHead = { 0 };
	tSPkgHead.flag = cHeadFlag;
	tSPkgHead.version = GetProtocolVersion();
	tSPkgHead.cmd = cmd;
	if(MSG_REQ == nSendType)
    {
        MakeSendPkgHead(tSPkgHead);
    }
    else if(MSG_RSP == nSendType)
    {
        MakeResponePkgHead(tSPkgHead);
    }
	memcpy(pBuffer, (char*)&tSPkgHead, nPkgHeadLen);

	//copy body
	if (NULL != input)
	{
		memcpy(pBuffer + nPkgHeadLen, (char*)input, inputlen);
	}

	//build package tail
	SPkgTail tSPkgTail = { 0 };
	tSPkgTail.crc = htons(CalcCRC1021((unsigned char*)(pBuffer + s32HeardOffSet), nAllocMemLen - s32HeardOffSet - nPkgTailLen)); //write crc
	tSPkgTail.flag = cTailFlag;
	memcpy(pBuffer + nPkgHeadLen + inputlen, (char *)&tSPkgTail,nPkgTailLen);

	//Whole package
	*output = tSPkgHead.flag;

	int&& ConvtLen = ConvtCode((unsigned char *)(pBuffer + s32HeardOffSet), nAllocMemLen - s32HeardOffSet - nTailFlagLen, (unsigned char *)(output + s32HeardOffSet));

	outputlen = s32HeardOffSet + ConvtLen + nTailFlagLen;         //2 is the len of '7d' + '0b'
	unsigned int&& u32PkgLen = htonl(outputlen);
	memcpy((char*)(output + nHeadFlagLen),(char*)&u32PkgLen,nIntLen);

	*(output + s32HeardOffSet + ConvtLen) = tSPkgTail.flag;

	//释放内存
	SafeDeleteArray(pBuffer);

	return true;
}

char* CNetTcpDataPkg::AllocateMem(const int nBodyBufferLen,int& nPkgLen)
{
	const int nBufferLen = nPkgHeadLen + nBodyBufferLen + nPkgTailLen;	//the len of whole package
	char* pBuffer = new char[nBufferLen + 1];
	memset(pBuffer, 0, nBufferLen + 1);
	nPkgLen = nBufferLen;
	return pBuffer;
}

unsigned char CNetTcpDataPkg::GetProtocolVersion()
{
	return m_u8Version;
}

void CNetTcpDataPkg::SetProtocolVersion(unsigned char u8Version)
{
	m_u8Version = u8Version;
}