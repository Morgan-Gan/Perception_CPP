#ifndef FACE_IDENTIFICATION_HPP_
#define FACE_IDENTIFICATION_HPP_
#include <semaphore.h>

namespace cv{
	template<typename _Tp> 
	  class Rect_;
	  typedef Rect_ <int> Rect2i;
	  typedef Rect2i Rect;
	  class  Mat;
}
class WaveID{
public:
	WaveID():face_net1_(nullptr),face_net2_(nullptr), feature_dim_(0), \
			img_h_(0), img_w_(0), gpu_id_(-1), \
			need_memory_(768), pSem_(nullptr){}
	~WaveID(){}

	//说明：初始化
	//参数：
	//	modelpath：模型所在目录路径
	//	pSem：加密狗
	//返回值：
	//	 0：成功
	//	-1：加密狗初始化失败
	//	-2：模型文件读取失败
	//	-3：模型解密空间开辟失败
	//	-4：网络重复初始化
	//	-5：模型文件解析失败
	//	-6：网络初始化失败
	int Initial(const char* modelpath, sem_t *pSem, int gpu_id);

	//说明：设置计算模式为CPU
	int SetModeCPU();

	//说明：设置计算模式为GPU
	int SetModeGPU(int gpu_id);

	//说明：释放
	//返回值：
	//	 0：成功
	//	-1：模型未初始化
	int Release();

	//说明：特征提取
	//参数：
	//	img：输入图像
	//	landmark：人脸关键点
	//	feature：输出特征
	//返回值：
	//	 0：特征提取成功
	int ExtractFeature(const cv::Mat img, const cv::Rect facerect,const cv::Mat landmark, \
			float* feature);

	//说明：获取模型提取的特征长度
	const int GetFeatureDim(){ return feature_dim_; }

	//说明：获取输入图像的高
	const int GetImgHeight(){ return img_h_; }

	//说明：获取输入图像的宽
	const int GetImgWidth(){ return img_w_; }
	
	//说明：获取当前设备ID，返回值若大于等于0则为GPU设备号，等于-1则为CPU模式
	const int GetDeviceID(){ return gpu_id_; }
	
	//说明：获取获取算法所需显存大小，单位Mb
	const int GetNeedMemory(){ return need_memory_; }

	//说明：特征相似度计算
	//参数：
	//	feature1：第一张人脸的特征
	//	feature2：第二张人脸的特征
	//返回值：
	//	相似度得分，取值为0~1之间
	double FaceVerify(const float* feature1, const float* feature2);

private:
	void* face_net1_;
	void* face_net2_;
	int feature_dim_;
	int feature_dim1_;
	int feature_dim2_;
	int img_h_;
	int img_w_;
	int gpu_id_;
	int need_memory_;

	sem_t *pSem_;
	static bool g_bDogOpened;
//	static int g_dogFd;
};

#endif

