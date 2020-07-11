#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <stdint.h>
#include "soapH.h"
#include "soapStub.h" 
#include "nsmap.h"
#include "onvif_client.h"
#include <uuid/uuid.h>

const int s32TimeOut = 5;
const int s32UUIDLen = 100;
char *was_To = "urn:schemas-xmlsoap-org:ws:2005:04:discovery";
char *was_Action = "http://schemas.xmlsoap.org/ws/2005/04/discovery/Probe";
char *multicast_addr = "soap.udp://239.255.255.250:3702";
char onvifUserName[100] = {0};
char onvifPassWord[100] = {0};

#if 1
//获取摄像头数据和时间
void GetSystemDataAndTime(struct soap *soap	,struct __wsdd__ProbeMatches *resp)
{
	 struct _tds__GetSystemDateAndTime SystemDateAndTime;
	 struct _tds__GetSystemDateAndTimeResponse SystemDateAndTimeResponse ;

	//鉴权
	soap_wsse_add_UsernameTokenDigest(soap,"user", onvifUserName, onvifPassWord);
	soap_call___tds__GetSystemDateAndTime(soap,resp->wsdd__ProbeMatches->ProbeMatch->XAddrs,NULL,&SystemDateAndTime,&SystemDateAndTimeResponse);
	if(soap->error)
	{
        int retval = soap->error;
        exit(-1) ;
	}
	else
	{
		if(SystemDateAndTimeResponse.SystemDateAndTime!=NULL)
		{
			//printf("DateTimeType::%d\n",SystemDateAndTimeResponse.SystemDateAndTime->DateTimeType);
		}
		else
		{
			//printf("Service Cap Get Inner Error\n");
		}
	}
}

//输入pc web的服务地址，获取摄像头ipc的媒体地址，如：http://172.168.0.216/onvif/media_service
void GetCapabilities(struct soap *soap	,struct __wsdd__ProbeMatches *resp,
		struct _tds__GetCapabilities *capa_req,struct _tds__GetCapabilitiesResponse *capa_resp)
{
    capa_req->Category = (enum tt__CapabilityCategory *)soap_malloc(soap, sizeof(int));
    capa_req->__sizeCategory = 1;
    *(capa_req->Category) = (enum tt__CapabilityCategory)(tt__CapabilityCategory__Media);

    capa_resp->Capabilities = (struct tt__Capabilities*)soap_malloc(soap,sizeof(struct tt__Capabilities)) ;
	soap_wsse_add_UsernameTokenDigest(soap,"user", onvifUserName, onvifPassWord);

    int result = soap_call___tds__GetCapabilities(soap, resp->wsdd__ProbeMatches->ProbeMatch->XAddrs, NULL, capa_req, capa_resp);
	if (soap->error)
    {
            int retval = soap->error;
            exit(-1) ;
    }
    else
    {
    	//printf(" \n--------------------GetCapabilities  OK! result=%d--------------\n \n",result);
        if(capa_resp->Capabilities==NULL)
        {
            //printf(" GetCapabilities  failed!  result=%d \n",result);
        }
        else
        {
            //printf(" Media->XAddr=%s \n", capa_resp->Capabilities->Media->XAddr);
        }
    }
}

//输入ipc的媒体地址，获取一个token值，如：Profile000
void GetProfiles(struct soap *soap,struct _trt__GetProfiles *trt__GetProfiles,
		struct _trt__GetProfilesResponse *trt__GetProfilesResponse ,struct _tds__GetCapabilitiesResponse *capa_resp)
{
	//printf("\n-------------------Getting Onvif Devices Profiles--------------\n\n");
	soap_wsse_add_UsernameTokenDigest(soap,"user", onvifUserName, onvifPassWord);
	int result = soap_call___trt__GetProfiles(soap, capa_resp->Capabilities->Media->XAddr, NULL, trt__GetProfiles, trt__GetProfilesResponse);
	
    //NOTE: it may be regular if result isn't SOAP_OK.Because some attributes aren't supported by server.
    if (result==-1)
	{
		result = soap->error;
		exit(-1);
	}
	else
	{
		//printf("\n-------------------Profiles Get OK--------------\n\n");
		if(trt__GetProfilesResponse->Profiles!=NULL)
		{
			if(trt__GetProfilesResponse->Profiles->Name!=NULL)
			{
				//printf("Profiles Name:%s  \n",trt__GetProfilesResponse->Profiles->Name);

			}
			if(trt__GetProfilesResponse->Profiles->token!=NULL)
			{
				//printf("Profiles Taken:%s\n",trt__GetProfilesResponse->Profiles->token);
			}
		}
		else
		{
			//printf("Profiles Get inner Error\n");
		}
	}
}

//输入一个token值，得到rtsp地址
void GetStreamUri(struct soap *soap,struct _trt__GetStreamUri *trt__GetStreamUri,struct _trt__GetStreamUriResponse *trt__GetStreamUriResponse,
		 struct _trt__GetProfilesResponse *trt__GetProfilesResponse,struct _tds__GetCapabilitiesResponse *capa_resp)
{
	trt__GetStreamUri->StreamSetup = (struct tt__StreamSetup*)soap_malloc(soap,sizeof(struct tt__StreamSetup));//初始化，分配空间
	trt__GetStreamUri->StreamSetup->Stream = 0;//stream type

	trt__GetStreamUri->StreamSetup->Transport = (struct tt__Transport *)soap_malloc(soap, sizeof(struct tt__Transport));//初始化，分配空间
	trt__GetStreamUri->StreamSetup->Transport->Protocol = 0;
	trt__GetStreamUri->StreamSetup->Transport->Tunnel = 0;
	trt__GetStreamUri->StreamSetup->__size = 1;
	trt__GetStreamUri->StreamSetup->__any = NULL;
	trt__GetStreamUri->StreamSetup->__anyAttribute =NULL;
	trt__GetStreamUri->ProfileToken = trt__GetProfilesResponse->Profiles->token ;

	soap_wsse_add_UsernameTokenDigest(soap,"user", onvifUserName, onvifPassWord);
	soap_call___trt__GetStreamUri(soap, capa_resp->Capabilities->Media->XAddr, NULL, trt__GetStreamUri, trt__GetStreamUriResponse);

	if (soap->error)
	{
		//printf("soap error: %d, %s, %s\n", soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
		int result = soap->error;
	}
	else
	{
		//printf("!!!!NOTE: RTSP Addr Get Done is :%s \n",trt__GetStreamUriResponse->MediaUri->Uri);
	}
}

struct soap* NewSoap(struct SOAP_ENV__Header *header,struct soap* soap,wsdd__ProbeType *req_,wsdd__ScopesType *sScope_)
{
	soap = soap_new();
	if(NULL == soap )
	{
		return NULL;
	}

	soap->recv_timeout = s32TimeOut;
	soap_set_namespaces(soap, namespaces);
	soap_default_SOAP_ENV__Header(soap, header);

	uuid_t uuid;
	char guid_string[s32UUIDLen];
	uuid_generate(uuid);
	uuid_unparse(uuid, guid_string);

	header->wsa__MessageID = guid_string;
	header->wsa__To = was_To;
	header->wsa__Action = was_Action;
	soap->header = header;

	soap_default_wsdd__ScopesType(soap, sScope_);
	sScope_->__item = "";
	soap_default_wsdd__ProbeType(soap, req_);
	req_->Scopes = sScope_;
	req_->Types = ""; //"dn:NetworkVideoTransmitter";

	return soap ;
}

void doSome(struct soap *soap,	struct SOAP_ENV__Header *header ,struct __wsdd__ProbeMatches *resp)
{
    GetSystemDataAndTime(soap,resp);

    struct _tds__GetCapabilities capa_req;
    struct _tds__GetCapabilitiesResponse capa_resp;
	GetCapabilities(soap,resp,&capa_req,&capa_resp);

	struct _trt__GetProfiles trt__GetProfiles;
	struct _trt__GetProfilesResponse trt__GetProfilesResponse ;
	GetProfiles(soap,&trt__GetProfiles,&trt__GetProfilesResponse,&capa_resp) ;

	struct _trt__GetStreamUri trt__GetStreamUri;
	struct _trt__GetStreamUriResponse trt__GetStreamUriResponse ;
	GetStreamUri(soap,&trt__GetStreamUri,&trt__GetStreamUriResponse,&trt__GetProfilesResponse,&capa_resp) ;
}

void DelSoap(struct soap* soap)
{
	soap_destroy(soap); 
	soap_end(soap); 
	soap_free(soap);
}

int ONVIF_GetIPCInformation(const char *ip, const char* username, const char* password, int channel, struct IpcInformation* ipc_info)
{  
    struct SOAP_ENV__Header header;  
    wsdd__ProbeType req;
	wsdd__ScopesType sScope;
    struct soap *soap;
	soap = NewSoap(&header,soap,&req,&sScope);
    if(NULL == soap)
    {
        return -1;
    }
	
    struct __wsdd__ProbeMatches resp;
	int result = soap_send___wsdd__Probe(soap, multicast_addr, NULL, &req);
	while(result == SOAP_OK)
	{
		result = soap_recv___wsdd__ProbeMatches(soap, &resp);
		if(result == SOAP_OK)
		{
			if(soap->error)  
			{
				result = soap->error;
			}
			else
			{
				for(int i = 0; i < resp.wsdd__ProbeMatches->__sizeProbeMatch; i++)
				{
                    /*
					printf("__sizeProbeMatch        : %d\r\n", resp.wsdd__ProbeMatches->__sizeProbeMatch);  
					printf("wsa__EndpointReference       : %p\r\n", resp.wsdd__ProbeMatches->ProbeMatch->wsa__EndpointReference);  
					printf("Target EP Address       : %s\r\n", resp.wsdd__ProbeMatches->ProbeMatch->wsa__EndpointReference.Address);  
					printf("Target Type             : %s\r\n", resp.wsdd__ProbeMatches->ProbeMatch->Types);  
					printf("Target Service Address  : %s\r\n", resp.wsdd__ProbeMatches->ProbeMatch->XAddrs);  
					printf("Target Metadata Version : %d\r\n", resp.wsdd__ProbeMatches->ProbeMatch->MetadataVersion);  
                    */
					if(resp.wsdd__ProbeMatches->ProbeMatch->Scopes)  
					{
						//printf("Target Scopes Address   : %s\r\n", resp.wsdd__ProbeMatches->ProbeMatch->Scopes->__item);
					}
				}
				break;
			}
		}
		else if (soap->error)
		{
			result = soap->error;
		}
	}

	if(result==SOAP_OK)
	{
		doSome(soap,&header,&resp);
	}
		
    DelSoap(soap);

	return result;  
}
#else
//初始化
static struct soap* ONVIF_Initsoap(struct SOAP_ENV__Header *header, const char *was_To, const char *was_Action, int timeout)
{
    struct soap *soap = NULL;    // soap环境变量
    unsigned char macaddr[6];
    char _HwId[1024];
    unsigned int Flagrand;
 
    soap = soap_new();
    if(soap == NULL)
    {
        printf("[%d]soap = NULL\n", __LINE__);
        return NULL;
    }
 
    soap_set_namespaces(soap, namespaces);   // 设置soap的namespaces，即设置命名空间
 
    // 设置超时（超过指定时间没有数据就退出）
    if(timeout > 0)
    {
        soap->recv_timeout = timeout;
        soap->send_timeout = timeout;
        soap->connect_timeout = timeout;
    }
    else
    {
        //Maximum waittime : 20s
        soap->recv_timeout  = 20;
        soap->send_timeout  = 20;
        soap->connect_timeout = 20;
    }
 
    soap_default_SOAP_ENV__Header(soap, header);
 
    //Create SessionID randomly,生成uuid(windows下叫guid，linux下叫uuid)，格式为urn:uuid:8-4-4-4-12，由系统随机产生
    srand((int)time(0));
    Flagrand = rand()%9000 + 8888;
    macaddr[0] = 0x1;
    macaddr[1] = 0x2;
    macaddr[2] = 0x3;
    macaddr[3] = 0x4;
    macaddr[4] = 0x5;
    macaddr[5] = 0x6;
    sprintf(_HwId, "urn:uuid:%ud68a-1dd2-11b2-a105-%02X%02X%02X%02X%02X%02X", Flagrand, macaddr[0], macaddr[1], macaddr[2],macaddr[3],macaddr[4],macaddr[5]);
    header->wsa__MessageID = (char *)malloc(100);  
    memset(header->wsa__MessageID, 0, 100);
    strncpy(header->wsa__MessageID, _HwId, strlen(_HwId));    //wsa__MessageID存放的是uuid
 
    if(was_Action != NULL)
    {
        header->wsa__Action = (char*)malloc(1024);
        memset(header->wsa__Action, '\0', 1024);
        strncpy(header->wsa__Action, was_Action, 1024); //
    }
    if(was_To != NULL)
    {
        header->wsa__To = (char *)malloc(1024);
        memset(header->wsa__To, '\0', 1024);
        strncpy(header->wsa__To, was_To, 1024);//"urn:schemas-xmlsoap-org:ws:2005:04:discovery";
    }
    soap->header = header;
    return soap;
}

//释放函数
static void ONVIF_soap_delete(struct soap *soap)
{
    soap_destroy(soap);                                                         // remove deserialized class instances (C++ only)
    soap_end(soap);                                                             // Clean up deserialized data (except class instances) and temporary data
    soap_free(soap);                                                            // Reset and deallocate the context created with soap_new or soap_copy
}

//鉴权
static int ONVIF_SetAuthInfo(struct soap *soap, const char *username, const char *password)
{
    int result = 0;
    if((NULL != username) || (NULL != password))
    {
        soap_wsse_add_UsernameTokenDigest(soap, NULL, username, password);
    }else{
        printf("un etAuth\n");
        result = -1;
    }
 
    return result;
}

//摄像头ipc的搜索,获取ipc web的服务地址列表
static int ONVIF_ClientDiscovery()
{
    //Create new soap object with info
    struct SOAP_ENV__Header header;
    struct soap* soap;
    soap = ONVIF_Initsoap(&header, was_To, was_Action, 10);
    soap_default_SOAP_ENV__Header(soap, &header);
    soap->header = &header;
    
    //设置Probe消息
    wsdd__ScopesType sScope;
    soap_default_wsdd__ScopesType(soap, &sScope);  					    // 设置寻找设备的范围
    sScope.__item = NULL;
    wsdd__ProbeType req;    			                                // 用于发送Probe消息
    soap_default_wsdd__ProbeType(soap, &req);  						    // 设置寻找设备的类型
    req.Scopes = &sScope;
    req.Types = NULL; //"dn:NetworkVideoTransmitter";
 
    //sent the message broadcast and wait
    int FoundDevNo = 0; 
    struct __wsdd__ProbeMatches resp;                                      // 用于接收Probe应答
    int retval = soap_send___wsdd__Probe(soap, multicast_addr, NULL, &req); // 向组播地址广播Probe消息
    while(retval == SOAP_OK)
    {
        retval = soap_recv___wsdd__ProbeMatches(soap, &resp);
        if(retval == SOAP_OK)
        {
            if(soap->error)
            {
                printf("[%d]:recv soap error :%d, %s, %s\n", __LINE__, soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
                retval = soap->error;
            }
            else //we find a device
            {
                FoundDevNo++;
                if(resp.wsdd__ProbeMatches->ProbeMatch != NULL && resp.wsdd__ProbeMatches->ProbeMatch->XAddrs != NULL)
                {
                    printf("***** No %d Devices Information *****\n", FoundDevNo);
                    printf("Device Service Address : %s\r\n", resp.wsdd__ProbeMatches->ProbeMatch->XAddrs);
                    printf("Device EP Address      : %s\r\n", resp.wsdd__ProbeMatches->ProbeMatch->wsa__EndpointReference.Address);
                    printf("Device Type            : %s\r\n", resp.wsdd__ProbeMatches->ProbeMatch->Types);
                    printf("Device Metadata Version: %d\r\n", resp.wsdd__ProbeMatches->ProbeMatch->MetadataVersion);
                    printf("[%d]*********************************\n", __LINE__);
                }
            }
        }
        else if(soap->error)
        {
            if(FoundDevNo == 0)
            {
                printf("No Device found!\n");
                retval = soap->error;
            }
            else
            {
                printf("Search end! Find %d Device! \n", FoundDevNo);
                retval = 0;
            }
            break;
        }
    }
 
    //释放函数
    ONVIF_soap_delete(soap);
    return retval;
}

//输入pc web的服务地址，获取摄像头ipc的媒体地址
static int ONVIF_GetCapabilities()
{
	int ret = 0;
    char sercer_addr[] = "http://172.168.0.211/onvif/device_service"; //设备搜索得到的地址
 
    struct SOAP_ENV__Header header;
    struct soap* soap = ONVIF_Initsoap(&header, NULL, NULL, 5);
 
    struct _tds__GetCapabilities *req;
    struct _tds__GetCapabilitiesResponse *Response;
    if(NULL == (req = (struct _tds__GetCapabilities *)calloc(1,sizeof(struct _tds__GetCapabilities))))
    {
        printf("calloc is error \n");
        ret = -1;
        return ret;
    }
	else
	{
        req->__sizeCategory = 1;
        req->Category = (enum tt__CapabilityCategory *)soap_malloc(soap, sizeof(int));
        *(req->Category) = (enum tt__CapabilityCategory)5; //5表示：tt__CapabilityCategory__Media
 
        ONVIF_SetAuthInfo(soap,"admin","hk123456");  //鉴权，输入摄像头的用户名、密码
        ret = soap_call___tds__GetCapabilities(soap, sercer_addr, NULL,req, Response);
        if(soap->error)
		{ 
            ret = -1;
            printf("soap error: %d, %s, %s\n", soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
            return ret;
        }
		else
		{
            if(NULL != Response->Capabilities)
            {
                if (Response->Capabilities->Media != NULL)
                {
                    if (Response->Capabilities->Media->XAddr != NULL)
                    {
                        printf(" media_addr: %s \n", Response->Capabilities->Media->XAddr); 
                    }
                }
            }
        }
    }
 
    if(NULL != req)
    {
        free(req);
        req = NULL;
    }

	ONVIF_soap_delete(soap);
	return ret;
}

//输入ipc web的服务地址，获取完整的摄像头ipc的媒体地址
//（有些h265的摄像头必须用到这个接口，得到获取能力时没获取到的另一个媒体地址）
static int  ONVIF_GetServices()
{
    int i = 0;
    int ret = 0;
    char secvre_addr[] = "http://172.168.0.211/onvif/device_service"; //设备搜索获取得到的服务地址
    struct SOAP_ENV__Header header;
    struct _tds__GetServices *tds__GetServices;
    struct _tds__GetServicesResponse *tds__GetServicesResponse;
 
    struct soap* soap = ONVIF_Initsoap(&header, NULL, NULL, 5);

    tds__GetServices->IncludeCapability = 0;
 
    ONVIF_SetAuthInfo(soap,"admin","123456");  //鉴权
    soap_call___tds__GetServices(soap,secvre_addr,NULL, tds__GetServices, tds__GetServicesResponse);
    if(soap->error)
	{ 
        ret = -1;
        printf("soap error: %d, %s, %s\n", soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
        return ret;
    }
	else
	{
        if (tds__GetServicesResponse->Service[i].Namespace != NULL )
		{
            for(i=0; i<tds__GetServicesResponse->__sizeService; i++)
            {
                if(strcmp(tds__GetServicesResponse->Service[i].Namespace, "http://www.onvif.org/ver20/media/wsdl") == 0)
                {
                    printf("  media_addr[%d] %s\n", i, tds__GetServicesResponse->Service[i].XAddr);
                }

                if(strcmp(tds__GetServicesResponse->Service[i].Namespace, "http://www.onvif.org/ver10/media/wsdl") == 0)     
                {
                    printf("  media_addr->XAddr[%d] %s\n", i, tds__GetServicesResponse->Service[i].XAddr);  
                }
            }

        }
    }
 
    ONVIF_soap_delete(soap);
	return ret;
}

static int ONVIF_GetProfiles()
{
   int i = 0;
    int ret = 0;
    char media_addr[] = "http://172.168.0.211/onvif/media_service";  //GetCapabilities得到的地址
    char media_addr2[] = "http://172.168.0.211/onvif/media2_service"; //GetServices得到的地址
    struct SOAP_ENV__Header header;  
    struct soap* soap = ONVIF_Initsoap(&header, NULL, NULL, 5);
 
    struct _trt__GetProfiles trt__GetProfiles;
    struct _trt__GetProfilesResponse trt__GetProfilesResponse;


    ONVIF_SetAuthInfo(soap,"admin","123456");  //鉴权
    soap_call___trt__GetProfiles(soap, media_addr, NULL, &trt__GetProfiles, &trt__GetProfilesResponse);   
  
    if(soap->error)
    { 
        ret = -1;
        printf("soap error: %d, %s, %s\n", soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
        return ret;
    }
	else
	{
        for(i=0; i<trt__GetProfilesResponse.__sizeProfiles; i++)
        {
            printf( "Profiles Name:%s  \n",trt__GetProfilesResponse.Profiles[i].Name); 
            printf( "Profiles Taken:%s\n",trt__GetProfilesResponse.Profiles[i].token);
        } 
    }
 
    ONVIF_soap_delete(soap);
    return ret;
}

static int ONVIF_GetStreamUri()
{
    int i = 0;
    int ret = 0;
	char media_addr[] = "http://172.168.0.211/onvif/media_service"; //GetCapabilities得到的地址
    char media_addr2[] = "http://172.168.0.211/onvif/media2_service"; //GetServices得到的地址
    char taken[] = "Profile000";   //get_profiles获取	
    struct SOAP_ENV__Header header;
	
    struct soap* soap = ONVIF_Initsoap(&header, NULL, NULL, 5);
 
    //...............................................h264通道....................................................
    struct _trt__GetStreamUri trt__GetStreamUri;
    struct _trt__GetStreamUriResponse response;
    trt__GetStreamUri.StreamSetup = (struct tt__StreamSetup*)soap_malloc(soap, sizeof(struct tt__StreamSetup));
    if (NULL == trt__GetStreamUri.StreamSetup){
        printf("soap_malloc is error\n");
		ret = -1;
    }
 
	trt__GetStreamUri.StreamSetup->Stream = tt__StreamType__RTP_Unicast;//stream type
	trt__GetStreamUri.StreamSetup->Transport = (struct tt__Transport *)soap_malloc(soap, sizeof(struct tt__Transport));
    if (NULL == trt__GetStreamUri.StreamSetup->Transport){
        printf("soap_malloc is error\n");
		ret = -1;
    }
 
	trt__GetStreamUri.StreamSetup->Transport->Protocol = 1;
	trt__GetStreamUri.StreamSetup->Transport->Tunnel = 0;
	trt__GetStreamUri.StreamSetup->__size = 1;
	trt__GetStreamUri.StreamSetup->__any = NULL;
	trt__GetStreamUri.StreamSetup->__anyAttribute = NULL;
 
    trt__GetStreamUri.ProfileToken = (char *)soap_malloc(soap, 128*sizeof(char ));//
    if (NULL == trt__GetStreamUri.ProfileToken){
        printf("soap_malloc is error\n");
		ret = -1;
    }
    strcpy(trt__GetStreamUri.ProfileToken, taken);
    ONVIF_SetAuthInfo(soap,"admin","123456");  //鉴权
    soap_call___trt__GetStreamUri(soap, media_addr, NULL, &trt__GetStreamUri, &response);
    if(soap->error)
    { 
        ret = -1;
        printf("soap error: %d, %s, %s\n", soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
        return ret;
    }
    else
    {
		printf("rtsp_addr: %s\n", response.MediaUri->Uri);
    }
	//...............................................h264通道....................................................
 
 
   //...............................................h265通道....................................................
   /*
    struct _tr2__GetStreamUri tr2__GetStreamUri;
	struct _tr2__GetStreamUriResponse tr2__GetStreamUriResponse;
    tr2__GetStreamUri.Protocol = (char *)soap_malloc(soap, 128*sizeof(char));//
    if (NULL == tr2__GetStreamUri.Protocol){
		printf("soap_malloc is error\n");
		ret = -1;
    }
    tr2__GetStreamUri.ProfileToken = (char *)soap_malloc(soap, 128*sizeof(char ));//
    if (NULL == tr2__GetStreamUri.ProfileToken){
        printf("soap_malloc is error\n");
		ret = -1;
    }
    strcpy(tr2__GetStreamUri.Protocol, "tcp");
    strcpy(tr2__GetStreamUri.ProfileToken, taken);
    ONVIF_SetAuthInfo(soap,"admin","123456");  //鉴权
	soap_call___tr2__GetStreamUri(soap, media_addr2, NULL, &tr2__GetStreamUri, &tr2__GetStreamUriResponse); 
    if(soap->error){ 
        ret = -1;
        printf("soap error: %d, %s, %s\n", soap->error, *soap_faultcode(soap), *soap_faultstring(soap));
        return ret;
    }else{
		printf("rtsp_addr: %s\n", tr2__GetStreamUriResponse.Uri);
    }*/
    //...............................................h265通道....................................................
 
    ONVIF_soap_delete(soap);
}

int test_onvif()
{
	//设备搜索
	if(ONVIF_ClientDiscovery() != 0)
    {
        printf("discover failed! \n");
        return -1;
    }

	//获取设备能力信息（获取媒体服务地址）
	if(0 != ONVIF_GetCapabilities)
	{
		return -1;
	}
	
	return 0;
}
#endif