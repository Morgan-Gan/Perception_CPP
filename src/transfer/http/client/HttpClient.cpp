#include "HttpClient.h"

using namespace std;
using namespace http_client;

const string strBoundary = "----Zn516969Mx0KrHhG0X99Yef9r8JZsRJSXC";
const string strMultiPartContentType = "multipart/form-data";
const string strJsonContentType = "application/json";

CHttpClient::CHttpClient(const string& strHost,int s32Port) :
m_strHost(strHost)
{
    m_strPort = std::to_string(s32Port);
}

int CHttpClient::Post(const string& strPage, const string& strReqData, string& strRspData)
{
	boost::asio::streambuf streamBuf;
	ostream ostreamReqPkg(&streamBuf);
    BuildCommReqPkg(strPage,strReqData,strJsonContentType,"POST",ostreamReqPkg);

	return PostOrGetData(streamBuf, strRspData);
}

int CHttpClient::PostMultiPart(const string& strPage,const vector<TMultiPartData>& lstReqData, string& strRspData)
{
	boost::asio::streambuf streamBuf;
	ostream ostreamReqPkg(&streamBuf);
    BuildMultiPartReqPkg(strPage,lstReqData,ostreamReqPkg);

	return PostOrGetData(streamBuf, strRspData);
}

int CHttpClient::Get(const string& strPage,const string& strReqData, string& strRspData)
{
    boost::asio::streambuf streamBuf;
	ostream ostreamReqPkg(&streamBuf);
    BuildCommReqPkg(strPage,strReqData,strJsonContentType,"GET",ostreamReqPkg);

	return PostOrGetData(streamBuf, strRspData);
}

int CHttpClient::PostOrGetData(boost::asio::streambuf& request, string & strRspData)
{
    try
    {
        boost::asio::io_service io_service;

        //如io_service存在复用的情况
        if (io_service.stopped())
        {
            io_service.reset();
        }

        //从dns获取域名下的所有ip
        tcp::resolver resolver(io_service);
        tcp::resolver::query query(m_strHost,m_strPort);
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
        
        //尝试连接到其中某个ip直到成功
        tcp::socket socket(io_service);
        boost::asio::connect(socket,endpoint_iterator);

        //发送请求
        boost::asio::write(socket, request);

        // Read the response status line. 
		boost::asio::streambuf response_data;
		boost::asio::read_until(socket, response_data, "\r\n");

        //响应校验
		istream stream_header(&response_data);
		string http_version;
		stream_header >> http_version;
		http_version = http_version.substr(0, 5);

        unsigned int status_code;
		stream_header >> status_code;

		string status_message;
		getline(stream_header, status_message);
		if (std::string("HTTP/") != http_version && std::string("OK") != status_message)    //版本信息校验
		{
			strRspData = "Invalid response";
			return -1;
		}
		
		if (200 != status_code)												                //状态码校验(返回非200都认为有错)
		{
			strRspData = "Response returned with status code != 200 ";
			return status_code;
		}

		//读包头数据
		string header;
		std::vector<string> headers;
		while (std::getline(stream_header, header) && header != "\r")
		{
			headers.push_back(header);
		}

		//读取所有剩下的数据作为包体
		boost::system::error_code error;
		while (boost::asio::read(socket, response_data,boost::asio::transfer_at_least(1), error))
		{
		}

		if (error != boost::asio::error::eof)
		{
			strRspData = error.message();
			return -1;
		}

		if (response_data.size() > 0)
		{
			std::istream stream_body(&response_data);
			std::istreambuf_iterator<char> eos;
			strRspData = string(std::istreambuf_iterator<char>(stream_body), eos);
		}
    }
    catch(const std::exception& e)
    {
        strRspData = e.what();
        std::cerr << e.what() << '\n';
        return -1;
    }
    
    return 0;
}

void CHttpClient::BuildCommReqPkg(const string& strPage,const string& strReqData, const string& strContentType,const string& strReqType,ostream& ostreamReqPkg)
{
    //构建整个包发送头
    BuildCommReqHeard(strPage, strContentType,strReqType,ostreamReqPkg);

    //拼接整个数据包
    ostreamReqPkg << "Content-Length: " << strReqData.length() << "\r\n\r\n";
    ostreamReqPkg << strReqData;
}

void CHttpClient::BuildMultiPartReqPkg(const string& strPage,const vector<TMultiPartData>& lstReqData, ostream& ostreamReqPkg)
{
    //构建整个包发送头
    BuildCommReqHeard(strPage, strMultiPartContentType,"POST",ostreamReqPkg);

    //构建由多部分(由头和体组成)组成的发送体
    stringstream streamBody;
    for(auto tReqData : lstReqData)
    {
        BuildMultiPartBody(tReqData,streamBody);
    }
    
    //构建整个包发送尾
    BuildMultiPartTail(streamBody);

    //拼接整个数据包
    ostreamReqPkg << "Content-Length: " << streamBody.str().length() << "\r\n\r\n";
    ostreamReqPkg << streamBody.str();
}

void CHttpClient::BuildCommReqHeard(const string& strPage,const string& strContentType,const string& strReqType,ostream& ostreamHeard)
{
        //构建发送头
		ostreamHeard << strReqType << " ";
		ostreamHeard << strPage << " HTTP/1.1\r\n";
		ostreamHeard << "Host: " << m_strHost << ":" << m_strPort << "\r\n";
		ostreamHeard << "Accept: */*\r\n";
		ostreamHeard << "Connection: close\r\n";
        ostreamHeard << "Content-Type: " << strContentType.c_str();
        (strMultiPartContentType == strContentType) ? ostreamHeard << "; boundary=" << strBoundary : ostreamHeard << "";
        ostreamHeard << "\r\n";
}

void CHttpClient::BuildMultiPartBody(const TMultiPartData& tReqData, stringstream& streamBody)
{
    //构建当前部分数据头部
	streamBody << "--" << strBoundary << "\r\n";
    streamBody << "Content-Disposition: form-data; name=" << tReqData.strName;
    if(tReqData.strFileName.empty())
    {
        streamBody << "; filename=" << tReqData.strFileName << "\r\n";
    }
    streamBody << "\r\n";

    if (!tReqData.strContentType.empty())
	{
        streamBody << "Content-Type: " << tReqData.strContentType << "\r\n";
	}

	if (!tReqData.strEncType.empty())
	{
		streamBody << "Content-Transfer-Encoding: " <<  tReqData.strEncType << "\r\n";
	}
	streamBody << "\r\n";

    //读当前部分数据体到输出流
    streamBody << tReqData.strData;
}

void CHttpClient::BuildMultiPartTail(stringstream& streamTail)
{
    streamTail << "--" << strBoundary << "--\r\n";
}