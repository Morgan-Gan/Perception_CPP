#include <string>
#include <vector>
#include "IVideoServiceModule.h"
#include "comm/Singleton.h"
#include "CfgMng/CfgMng.h"

namespace MAIN_MNG
{
    class CVsTransferMng : public IF_VIDEO_SERVICE::IVideoStreamCallback,
    public common_template::CSingleton<CVsTransferMng>
    {
        friend class common_template::CSingleton<CVsTransferMng>;

    public:
        //启动摄像
        void StartCameras();

        //摄像机运行状态上报
        virtual bool ReportCameraRunState(const std::string& CameraCode, const int state);

    private:
        CVsTransferMng(){};
        ~CVsTransferMng(){};
    };
    #define SCVsTransferMng (common_template::CSingleton<CVsTransferMng>::GetInstance())
}