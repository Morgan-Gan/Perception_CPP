#include "face_alignment.h"
#include <opencv2/core/core.hpp>    
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <stdio.h>
#include <GpuAlloc.h>
using namespace cv;
int main(int argc, char** argv)
{
	argv[1] = "./data/image_00000009.jpg";
	argv[2] = "../libwaveface_alignment/models";
	FaceAlignment face_alignment;
        int gpu_id = AllocGPUId(face_alignment.GetNeedMemory());
        for(int i=0;i<50;i++){
	int ret = face_alignment.Initial(argv[2],gpu_id);

	//int ret = face_alignment.Initial(argv[2],0);
        printf("gpu_id :%d\n",gpu_id);
	if (ret)
	{
		printf("face_alignment error\n");
	}
	else
	{
		printf("face_alignment success\n");
	}
	Rect faceRect;
	faceRect.x = 65;
	faceRect.y = 125;
	faceRect.width = 160;
	faceRect.height = 160;
	Mat img = imread(argv[1]);
	//float* landmarks = NULL;
	cv::Mat shape_src;
	ret = face_alignment.Alignment(img, faceRect,&shape_src );
	if (ret)
	{
		printf("face_alignment.Alignment failed\n");
		face_alignment.Release();
		return -1;
	}
	else
	{
		printf("face_alignment.Alignment success\n");

	}
//float landmarks[]={95.683365,149.825226,96.295074,170.316269,99.217453,190.874252,104.208923,211.090485,111.670990,230.120544,122.511902,247.077972,136.036774,261.423920,151.431854,273.118866,169.973145,277.200012,190.064850,274.133484,209.644836,263.851135,227.183746,250.057541,241.423706,232.871582,250.755615,212.616394,255.822403,190.455276,258.438232,167.244415,258.953186,144.257935,101.620102,133.840195,111.155746,127.324463,123.893509,127.374313,136.675919,131.309006,148.831390,136.688263,177.511230,136.130630,191.341232,129.940002,206.666000,125.934586,222.643478,126.184525,236.059799,132.959137,162.888168,154.750992,162.186676,168.201645,161.262863,181.271149,160.363068,194.628967,147.082626,206.167374,154.375595,208.780762,162.974045,210.301468,172.368988,208.644302,181.351074,206.388763,115.144989,156.394791,124.006775,153.713806,134.258698,154.046829,143.842407,158.722488,133.666962,161.544281,123.591400,161.225601,188.948608,158.496628,198.145752,153.550537,209.170776,153.202164,220.098953,156.010178,209.914841,160.854828,198.959167,161.228668,140.425140,233.697098,148.388290,228.118698,157.373383,224.742554,164.347534,226.988174,171.713791,224.707352,183.633072,228.449341,195.834457,234.029297,184.466217,243.438950,173.201797,247.951141,164.752045,248.655319,156.590851,247.306305,148.242447,242.461365,144.712708,233.948837,157.275391,233.175858,164.674728,234.157928,172.356506,233.291046,191.109268,234.181396,172.372040,235.775742,164.448364,236.425903,157.057465,235.394989};
	float* landmarks =(float*) shape_src.data;
        FILE  * fp = fopen("1.txt","w+");
	for (int i = 0; i < 68; i++)
	{
		fprintf(fp,",%f,%f",landmarks[2 * i],landmarks[2 * i+1]);
		shape_src.at<float>(0, 2 * i) = landmarks[2 * i];
                shape_src.at<float>(0, 2 * i + 1) = landmarks[2 * i+1];
		circle(img, Point(int(landmarks[2 * i]), int(landmarks[2 * i + 1])), 2, CV_RGB(255, 0, 0), 2);
	}
	fclose(fp);
	printf("face_alignment.Release end\n");
rectangle(img,faceRect,Scalar(0,255,0),2);
	imwrite("1.bmp",img);
         face_alignment.Release();
        }
	return 0;
}
