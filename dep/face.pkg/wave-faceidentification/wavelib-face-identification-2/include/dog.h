/**
*���·�װlkt5103�ӿڣ����õ���ģʽ����Ϊ����ģ�飬�Թ�����������Ϊ�㷨ģ��ͬʱ���̵߳���
*������ϵ��liangxianyang@wavewisdom.com
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
	*���ܣ���ʼ�����ܹ�
	*����ֵ��0-�ɹ�������-������
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

