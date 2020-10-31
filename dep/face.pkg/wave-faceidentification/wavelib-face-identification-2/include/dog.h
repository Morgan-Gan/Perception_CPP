/**
*重新封装lkt5103接口，采用单例模式，作为公共模块，以供各人脸、行为算法模块同时多线程调用
*问题联系：liangxianyang@wavewisdom.com
*/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "lkt5103.h"
#include <stdbool.h>
#include "GpuAlloc.h"

typedef unsigned char byte;

class DogManagerSingleton
{
public:
	DogManagerSingleton();
	~DogManagerSingleton();

	/**
	*功能：初始化加密狗
	*返回值：0-成功，其他-错误码
	*/
	static int InitDog();

	static int DogDecrypt_3des(byte* data, int lenOfData, unsigned char *resBuf, int * lenOfRes);

	static int CloseDog();

	static GpuManagerSingleton * GetInstance();

private:
	static GpuManagerSingleton * instance_;
	static int TryOpenDog(char* dogPath, int& SC_Handle);
	static std::mutex dogMutex_;
	static bool bDogOpened_;
	static int dogFd_;
};

