#include "main.h"
#include "DbMng/DbMng.h"
#include "TransferMng/TransferMng.h"
#include "CfgMng/CfgMng.h"
#include "AlgMng/AlgModuleMng.h"
#include "AppMng/AppModuleMng.h"

using namespace std;
using namespace MAIN_MNG;

int main(int argc, char **argv)
{
    /*设置系统信号*/
	signal(SIGINT, &catch_quit);
	signal(SIGTERM, &catch_quit);
	signal(SIGKILL, &catch_quit);
	signal(SIGABRT, &catch_quit);
	signal(SIGSTOP, &catch_quit);
	signal(SIGQUIT, &catch_quit);

    /*配置加载初始化*/
	if (!SCCfgMng.LoadCfg())
	{
		return 0;
	}

    /*数据库初始化*/
	SCDbMng.InitDb();

	/*北向通信启动*/
	SCTransferMng.StartNorthTrans();

	/*加载算法模块*/
	SCAlgModuleMng.StartAlgModule();

	/*加载应用模块*/
	SCAppModuleMng.StartAppModule();

	/*南向通信启动*/
	SCTransferMng.StartSouthTrans();
}
