
#include "dog.h"

#include<unistd.h>
#include<string.h>


bool DogManagerSingleton::bDogOpened_ = false;
int DogManagerSingleton::dogFd_ = 0;
GpuManagerSingleton * DogManagerSingleton::instance_ ;
std::mutex DogManagerSingleton::dogMutex_;

int DogManagerSingleton::InitDog()
{
	dogMutex_.lock();
	if (bDogOpened_)  //只打开一次，直接返回成功
	{
		dogMutex_.unlock();
		return 0;
	}

	char dogPath[256];

	//search in sgx
	for (int i = 0; i < 20; ++i)
	{
		sprintf(dogPath, "/dev/sg%d", i);
		int ret = TryOpenDog(dogPath, dogFd_);
		if (ret == 0)
		{
			bDogOpened_ = true;
			dogMutex_.unlock();
			return 0;
		}
	}

	//search in sdb
	sprintf(dogPath, "/dev/sdb");
	int ret = TryOpenDog(dogPath, dogFd_);
	if (ret == 0)
	{
		bDogOpened_ = true;
		dogMutex_.unlock();
		return 0;
	}

	dogMutex_.unlock();
	return ret;
}

int DogManagerSingleton::TryOpenDog(char* dogPath, int& SC_Handle)
{
	printf("Open %s\n", dogPath);

	char cmd[256];
	sprintf(cmd, "chmod 777 %s", dogPath);
	int s = system(cmd);
	printf("chmod = %d\n", s);

	int ret = LKSC_Open(dogPath, &SC_Handle);
	if (ret < 0)
	{
		printf("Open error!\n");
		return ret;
	}
	else
	{
		printf("LKSC_Open OK !\n");
	}
	unsigned char ATR[0X40];
	int LenOfAtr = 0;
	try{
		ret = LKSC_Reset(SC_Handle, ATR, &LenOfAtr);
	}
	catch (...)
	{
		return -1;
	}

	printf("ATR Len = %d \n", LenOfAtr);

	if (LenOfAtr != 17) return -2;

	printf("ATR = ");
	for (int i = 0; i < LenOfAtr; i++)
	{
		printf("%.2X ", ATR[i]);
	}
	printf("\n");

	return 0;
}


int DogManagerSingleton::DogDecrypt_3des(byte* data, int lenOfData, unsigned char *resBuf, int * lenOfRes)
{
	if (lenOfData > 256)
	{
		printf("LenOfData  = %d, too long\n", lenOfData);
		return -1;
	}
	if (lenOfData % 8)
	{
		printf("Data is not aligned\n");
		return -2;
	}
	
	if (!dogMutex_.try_lock())
	{
		//printf("Dog is busy\n");
		return -3;
	}

	//decrypt_3des by dog
	unsigned char cmd[300] = { 0x80, 0x08, 0x00, 0x00, 0xFA, 0x06, 0xF8 };
	memcpy(cmd + 7, data, lenOfData);
	int ret_lkt = LKSC_SendAPDU(dogFd_, cmd, lenOfData + 7, resBuf, lenOfRes);
	if (ret_lkt)
	{
		printf("LKSC_SendAPDU error: %d!\n", ret_lkt);
		dogMutex_.unlock();
		return -3;
	}
	
	//unlocked
	dogMutex_.unlock();
	return 0;
}

int DogManagerSingleton::CloseDog()
{
	dogMutex_.lock();
	if (bDogOpened_)
	{
		//close dog
		LKSC_Close(dogFd_);
		bDogOpened_ = false;  //only close dog one time
	}
	dogMutex_.unlock();
}


GpuManagerSingleton * DogManagerSingleton::GetInstance()
{
	//instance_ = GpuManagerSingleton::GetInstance();
	return instance_;
}

DogManagerSingleton::DogManagerSingleton()
{
}

DogManagerSingleton::~DogManagerSingleton()
{
}
