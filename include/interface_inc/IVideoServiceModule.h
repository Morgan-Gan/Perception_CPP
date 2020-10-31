#pragma once
#include <string>
#include <map>
//opencv的头文件
#include <opencv2/opencv.hpp>
//boost的头文件
#include <boost/mpl/int.hpp>
#include <mutex>

namespace IF_VIDEO_SERVICE
{       
    const int gCameraStateReportInterval = 1;					//摄像头上报状态间隔(秒)
    const int gActDiscernInterval = 1;							//摄像头行为识别间隔(秒)
 
    const int gMaxTaskNum = 256;                                //最大任务数
    const int gBitNum = 8;                                      //一个字符Bit位数

    #pragma pack(push, 1)  
    typedef struct
    {
        int64_t s64Id;
        int32_t s32Port;
        int32_t s32GetStreamMode;
        int32_t s32State;
        int32_t s32CoordinateX;
        int32_t s32CoordinateY;
        int32_t s32Width;
        int32_t s32Height;
        int32_t s32IsReport;
        int32_t s32OffOrOn;
        std::string strCameraCode;
        std::string strCameraName;
        std::string strDiscernSysCode;
        std::string strDoorCode;
        std::string strIP;
        std::string strMac;
        std::string strUserName;
        std::string strPassword;
        std::string strProtocol;
        std::string strCameraPixel;
        std::string strTaskSet;
        std::string strImageGroupSet;
        std::string strInstallAddr;
        std::string strStartTime;
        std::string strEndTime;
        std::string strCreateTime;
        std::string strRegion;
        std::string strCameraLiveId;
        std::string strRemark;
    }TCameraBaseInfo;

    //视频流结构体
    typedef struct st_cvMat 
    {
        std::string strShotTime;
        cv::Mat srcMat;
    }cvtMat;

    //行为类型
    enum class eActType
    {
        NonSelfDef = 1,
        SelfDef = 2
    };
    #pragma pack(pop)

/*摄像机拉流*/
    using VecCamBaseInfo = std::vector<IF_VIDEO_SERVICE::TCameraBaseInfo>;
    using MapCamBaseInfo = std::map<std::string, IF_VIDEO_SERVICE::TCameraBaseInfo>;
    class IVideoStreamCallback
    {
    public:
        //上报摄像机运行状态
        virtual bool ReportCameraRunState(const std::string& CameraCode, const int32_t state) = 0;
    };

    class IVideoStreamModule
    {
    public:
        virtual bool InitModule(IVideoStreamCallback* pCallbackObj) = 0;
        virtual bool StartModule(const MapCamBaseInfo& mapCamera) = 0;

        //从行为队列获取图片
        virtual void GetActImage(const std::string& strCameraCode, eActType AccType, st_cvMat& stcvMat) = 0;

        //从人脸队列获取图片
        virtual void GetCmmImage(const std::string& strCameraCode, st_cvMat& stcvMat) = 0;

        //上报摄像机运行状态
        virtual bool ReportCameraRunState(const std::string& CameraCode, const int32_t state) = 0;

        //同步添加摄像机数据
        virtual void SyncAddCameraInfo(const std::vector<TCameraBaseInfo>& vecCameraBaseInfo) = 0;

        //同步删除摄像机数据
        virtual void SyncDelCameraInfo(const std::vector<TCameraBaseInfo>& vecCameraBaseInfo) = 0;

        //同步修改摄像机数据
        virtual void SyncModifyCameraInfo(const std::vector<TCameraBaseInfo>& vecCameraBaseInfo) = 0;
    };
    extern "C" IVideoStreamModule* GetVSModuleInstance();

/*摄像机信息同步*/
    typedef struct
    {
    }st_ImageSampleInfo;

    typedef struct
    {
    }st_CameraRelationTableInfo;

    typedef struct
    {
    }st_CameraTableKeyInfo;

    typedef struct
    {
    }st_ImageGroupInfo;

    typedef struct
    {
    }st_DiscernTaskDef;

    typedef struct
    {
    }st_DBInstanceInfo;

    typedef struct
    {
    }st_DiscernSysInfo;

    using MapImageSampleInfo = std::map<int, std::vector<st_ImageSampleInfo> >;
    using MapCameraRelationTableInfo = std::map<std::string, std::vector<st_CameraRelationTableInfo> >;
    using MapCameraBaseInfo = std::map<std::string, TCameraBaseInfo>;
    using VecCameraTableKeyInfo = std::vector<st_CameraTableKeyInfo>;
    using VecCameraRelationTableInfo = std::vector<st_CameraRelationTableInfo>;
    using VecImageGroupInfo = std::vector<st_ImageGroupInfo>;
    using VecCameraBaseInfo = std::vector<TCameraBaseInfo>;
    using VecImageSampleInfo = std::vector<st_ImageSampleInfo>;
    using VecDiscernTaskDef = std::vector<st_DiscernTaskDef>;
    using StDiscernTaskDef = st_DiscernTaskDef;
    class ICSysDataCallback
    {
    public:
        //注册站点
        virtual void RegDiscernSys(const std::string& strDisSysCode, const st_DiscernSysInfo& InDiscernSysInfo, st_DiscernSysInfo& OutDiscernSysInfo) = 0;

        //加载数据库实例
        virtual void LoadDBInstance(const std::string strDBInstanceCode, st_DBInstanceInfo& stDBInstanceInfo) = 0;

        //加载摄像机表关键字列表
        virtual void LoadCameraTableKeyList(VecCameraTableKeyInfo& vecCamTabKeyInfo) = 0;

        //加载摄像机列表信息
        virtual void LoadCameraList(VecCameraBaseInfo& vecCamBaseInfo) = 0;

        //同步增加摄像机信息
        virtual void SyncAddCameraInfo(VecCameraBaseInfo& vecCamBaseInfo) = 0;

        //同步删除摄像机信息
        virtual void SyncDelCameraInfo(VecCameraBaseInfo& vecCamBaseInfo) = 0;

        //同步修改摄像机信息
        virtual void SyncModifyCameraInfo(VecCameraBaseInfo& vecCamBaseInfo) = 0;

        //上报摄像机相关表信息
        virtual bool ReportCameraRelationTable(const VecCameraRelationTableInfo& vecCamRelaTabInfo) = 0;

        //上报修改摄像机相关表信息
        virtual bool ReportModifyCameraTable(const VecCameraRelationTableInfo& vecCamRelaTabInfo) = 0;

        //加载摄像机表关联表列表
        virtual void LoadCameraTableList(const std::string StartTime, MapCameraRelationTableInfo& mapCamRelaTabInfo) = 0;

        //加载人像样本组信息列表
        virtual void LoadImageGroupList(VecImageGroupInfo& vecImageGroupInfo) = 0;

        //加载人像样本列表（按组加截）
        virtual void LoadImageSampleMap(MapImageSampleInfo& mapImageSampleInfo) = 0;

        //加载某一组的人像样本列表
        virtual void LoadImageSampleList(const int ImageGroupNum, VecImageSampleInfo& vecImageSampleInfo) = 0;

        //同步增加人像样本组信息
        virtual void SyncAddImageGroup(VecImageGroupInfo& vecImageSampleInfo) = 0;

        //同步删除人像样本组信息
        virtual void SyncDelImageGroup(VecImageGroupInfo& vecImageSampleInfo) = 0;

        //同步修改人像样本组信息
        virtual void SyncModifyImageGroup(VecImageGroupInfo& vecImageSampleInfo) = 0;

        //同步人像样本列表（按组同步）
        virtual void GetImageSampleMap(MapImageSampleInfo& mapImageSampleInfo) = 0;

        //同步增加的人像样本信息
        virtual void SyncAddImageSample(MapImageSampleInfo& mapImageSampleInfo) = 0;

        //同步删除的人像样本信息
        virtual void SyncDelImageSample(MapImageSampleInfo& mapImageSampleInfo) = 0;

        //同步修改的人像样本信息
        virtual void SyncModifyImageSample(MapImageSampleInfo& mapImageSampleInfo) = 0;

        //获取任务列表信息
        virtual void GetDiscernTaskList(VecDiscernTaskDef& vecDiscernTaskDef) = 0;

        //同步添加任务列表信息
        virtual void SyncAddDiscernTask(VecDiscernTaskDef& vecDiscernTaskDef) = 0;

        //同步删除任务列表信息
        virtual void SyncDelDiscernTask(VecDiscernTaskDef& vecDiscernTaskDef) = 0;

        //同步修改任务列表信息
        virtual void SyncModifyDiscernTask(VecDiscernTaskDef& vecDiscernTaskDef) = 0;
    };

    class ICSysDataModule
    {
    public:
        virtual bool StartModule(const std::string& strEventNum) = 0;
        virtual bool SetCallbackObj(ICSysDataCallback* pCallbackObj) = 0;

        //获取摄像头列表
        virtual void GetCameraLst(MapCameraBaseInfo& vecCamBaseInfo) = 0;

        //获取数据库实例
        virtual void GetDBInstanceInfo(st_DBInstanceInfo& stDBInstanceInfo) = 0;

        //获取人脸样本列表
        virtual void GetImageSampleLst(MapImageSampleInfo& mapImageSampleInfo) = 0;

        //获取识别表信息
        virtual void GetTableInfoList(const std::string& strTabKey, const std::string& strCamCodeSet, std::string& strDisSTime, std::string& strDisETime, std::vector<std::string>& vecQueryTable) = 0;

        //获取行为识别最新表
        virtual void GetActLastTableName(const std::string& strCamCode, std::string& strTabName) = 0;

        //获取人脸识别最新表
        virtual void GetFaceLastTableName(const std::string& strCamCode, std::string& strTabName) = 0;

        //获取某个位任务信息
        virtual bool GetDiscernTaskByNum(const int32_t nTaskNum, StDiscernTaskDef& stDiscernTaskDef) = 0;
    };
    extern "C" ICSysDataModule* GetSysDataModuleInstance();
}