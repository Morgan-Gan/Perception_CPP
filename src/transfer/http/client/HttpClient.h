#pragma once
#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <vector>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

namespace http_client
{
    typedef struct
    {
        std::string strName;
        std::string strFileName;
        std::string strContentType;
        std::string strEncType;
        std::string strData;
    }TMultiPartData;

    class CHttpClient
	{
	public:
		CHttpClient() = default;
        CHttpClient(const std::string& strHost,int s32Post);
		~CHttpClient() = default;

    public:
    	int Post(const std::string& strPage, const std::string& strReqData, std::string& strRspData);
		int PostMultiPart(const std::string& strPage,const std::vector<TMultiPartData>& lstReqData, std::string& strRspData);
        int Get(const std::string& strPage,const std::string& strReqData, std::string& strRspData);

	private:
		int PostOrGetData(boost::asio::streambuf& streamReqData, std::string& strRspData);
        void BuildCommReqPkg(const std::string& strPage,const std::string& strReqData, const std::string& strContentType,const std::string& strReqType, std::ostream& ostreamReqPkg);
        void BuildMultiPartReqPkg(const std::string& strPage,const std::vector<TMultiPartData>& lstReqData, std::ostream& ostreamReqPkg);
        void BuildCommReqHeard(const std::string& strPage,const std::string& strContentType,const std::string& strReqType,std::ostream& ostreamHeard);
        void BuildMultiPartBody(const TMultiPartData& tReqData, std::stringstream& streamBody);
        void BuildMultiPartTail(std::stringstream& streamTail);

    private:
        std::string m_strHost;
        std::string m_strPort;
    };
}