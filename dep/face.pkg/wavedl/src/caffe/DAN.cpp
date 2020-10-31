#include "caffe/DAN.h"
#define IMGSIZE  112
#define LANDMARK  68
#define MIRRORS   1
#define DATASCALE  10
#define  PI   3.1415926
int GetAffineParam(float*ShapesFrom, float* ShapeTo, Mat& T, Mat& C)
{
	if (ShapesFrom == NULL || ShapeTo == NULL)
	{
		return -1;
	}
	float dest_mean_x = 0;
	float dest_mean_y = 0;
	float src_mean_x = 0;
	float src_mean_y = 0;
	float* pData = ShapeTo;
	for (int i = 0; i < LANDMARK; i++)
	{
		dest_mean_x += *pData;
		pData++;
		dest_mean_y += *pData;
		pData++;
	}
	dest_mean_x /= LANDMARK;
	dest_mean_y /= LANDMARK;
	pData = ShapesFrom;
	for (int i = 0; i < LANDMARK; i++)
	{
		src_mean_x += *pData;
		pData++;
		src_mean_y += *pData;
		pData++;
	}
	src_mean_x /= LANDMARK;
	src_mean_y /= LANDMARK;
	int len = LANDMARK * 2;
	float *srcVec = new float[len];
	float *destVec = new float[len];
	pData = ShapesFrom;
	float* pData1 = srcVec;
	for (int i = 0; i < LANDMARK; i++)
	{
		*pData1 = *pData - src_mean_x;
		pData++;
		pData1++;
		*pData1 = *pData - src_mean_y;
		pData++;
		pData1++;
	}
	pData = ShapeTo;
	pData1 = destVec;
	for (int i = 0; i < LANDMARK; i++)
	{
		*pData1 = *pData - dest_mean_x;
		pData++;
		pData1++;
		*pData1 = *pData - dest_mean_y;
		pData++;
		pData1++;
	}
	float sum1 = 0;
	float sum2 = 0.0;
	pData = srcVec;
	pData1 = destVec;
	for (int i = 0; i < len; i++)
	{
		sum1 += (*pData)*(*pData1);
		sum2 += (*pData)*(*pData);
		pData++;
		pData1++;
	}
	float a = sum1 / sum2;
	float b = 0;
	pData = srcVec;
	pData1 = destVec;
	for (int i = 0; i < LANDMARK; i++)
	{
		b += pData[0] * pData1[1] - pData[1] * pData1[0];
		pData += 2;
		pData1 += 2;
	}
	b = b / sum2;
	T.create(2, 2, CV_32FC1);
	pData = (float*)T.data;
	pData[0] = a;
	pData[1] = b;
	pData[2] = -b;
	pData[3] = a;
	float c1 = src_mean_x*a - b*src_mean_y;
	float c2 = b*src_mean_x + a*src_mean_y;
	C.create(1, 2, CV_32FC1);
	pData = (float*)C.data;
	pData[0] = dest_mean_x - c1;
	pData[1] = dest_mean_y - c2;
	delete[] srcVec;
	delete[]  destVec;
	return 0;
}
int generratePixels(float* pixels)
{
	float* pData = pixels;
	for (int i = 0; i < IMGSIZE; i++)
	{
		for (int j = 0; j < IMGSIZE; j++)
		{
			pData[0] = i;
			pData[1] = j;
			pData += 2;
		}
	}
	return 0;
}
int AffineImage(Mat Image, const Mat& R, const Mat& C,Mat& outImage, bool isInv)
{
	Mat A, T;
	if (isInv == true)
	{
		A = R.inv();
		
		T.create(1, 2, CV_32FC1);
		float *pData = (float*)T.data;
		float *pData1 = (float*)C.data;
		float* pData2 = (float*)A.data;
		pData[0] = (pData1[0] * pData2[0] + pData1[1] * pData2[2])*(-1.0);
		pData[1] = (pData1[0] * pData2[1] + pData1[1] * pData2[3])*(-1.0);
	}
	else
	{
		R.copyTo(A);
		C.copyTo(T);
	}
	Mat ImgVec;
	if (Image.channels()==1)
	{
		
		ImgVec.create(IMGSIZE, IMGSIZE, CV_8UC1);
		unsigned char* ImageDst = (unsigned char*)ImgVec.data;
		unsigned char* ImageSrc = (unsigned char*)Image.data;
		float* pData2 = (float*)A.data;
		float* pData1 = (float*)T.data;
		for (int i = 0; i < IMGSIZE; i++)
		{

			for (int j = 0; j < IMGSIZE; j++)
			{
				float x1 = j * pData2[0] + i * pData2[2] + pData1[0];
				float y1 = j * pData2[1] + i * pData2[3] + pData1[1];
				int x = x1;
				int y = y1;

				if (x < 1 || x >= Image.cols - 1 || y < 1 || y >= Image.rows - 1)
				{
					ImageDst[i*ImgVec.step[0] + j] = 0;
				}
				else
				{
					unsigned char* p = ImageSrc + y*Image.step[0];
					unsigned char* p1 = ImageSrc + (y + 1)*Image.step[0];
					float f1x = p[x] * (1 - (x1 - x)) + p[(x + 1)] * (x1 - x);
					float f2x = p1[x] * (1 - (x1 - x)) + p1[(x + 1)] * (x1 - x);
					ImageDst[i*ImgVec.step[0] + j] = f1x*(1 - (y1 - y)) + f2x*((y1 - y));
					
				}

			}
		}
		
	}
	else
	{
		ImgVec.create(IMGSIZE, IMGSIZE, CV_8UC3);
		unsigned char* ImageDst = (unsigned char*)ImgVec.data;
		unsigned char* ImageSrc = (unsigned char*)Image.data;
		float* pData2 = (float*)A.data;
		float* pData1 = (float*)T.data;
		for (int i = 0; i < IMGSIZE; i++)
		{

			for (int j = 0; j < IMGSIZE; j++)
			{
				float x1 = j * pData2[0] + i * pData2[2] + pData1[0];
				float y1 = j * pData2[1] + i * pData2[3] + pData1[1];
				int x = x1;
				int y = y1;
				if (x < 1 || x >= Image.cols - 1 || y < 1 || y >= Image.rows - 1)
				{
					ImageDst[i*ImgVec.step[0] + 3 * j] = 0;
					ImageDst[i*ImgVec.step[0] + 3 * j + 1] = 0;
					ImageDst[i*ImgVec.step[0] + 3 * j + 2] = 0;
				}
				else
				{
					int channel = 3;
					unsigned char* p = ImageSrc + y*Image.step[0];
					unsigned char* p1 = ImageSrc + (y + 1)*Image.step[0];
					float f1x = p[x * channel] * (1 - (x1 - x)) + p[(x + 1) * channel] * (x1 - x);
					float f1y = p[x * channel + 1] * (1 - (x1 - x)) + p[(x + 1) * channel + 1] * (x1 - x);
					float f1z = p[x * channel + 2] * (1 - (x1 - x)) + p[(x + 1) * channel + 2] * (x1 - x);
					float f2x = p1[x * channel] * (1 - (x1 - x)) + p1[(x + 1) * channel] * (x1 - x);
					float f2y = p1[x * channel + 1] * (1 - (x1 - x)) + p1[(x + 1) * channel + 1] * (x1 - x);
					float f2z = p1[x * channel + 2] * (1 - (x1 - x)) + p1[(x + 1) * channel + 2] * (x1 - x);
					ImageDst[i*ImgVec.step[0] + 3 * j] = f1x*(1 - (y1 - y)) + f2x*((y1 - y));
					ImageDst[i*ImgVec.step[0] + 3 * j + 1] = f1y*(1 - (y1 - y)) + f2y*((y1 - y));
					ImageDst[i*ImgVec.step[0] + 3 * j + 2] = f1z*(1 - (y1 - y)) + f2z*((y1 - y));
				}

			}
		}
	}
	ImgVec.copyTo(outImage);
	return 0;
}


int AffinefloatImage(float* Image,int nheight, int nwidth,int nchannels, const Mat& R, const Mat& C, float* outImage,float value, bool isInv)
{
	Mat A, T;
	if (isInv == true)
	{
		A = R.inv();
		T.create(1, 2, CV_32FC1);
		float *pData = (float*)T.data;
		float *pData1 = (float*)C.data;
		float* pData2 = (float*)A.data;
		pData[0] = (pData1[0] * pData2[0] + pData1[1] * pData2[2])*(-1.0);
		pData[1] = (pData1[0] * pData2[1] + pData1[1] * pData2[3])*(-1.0);
	}
	else
	{
		R.copyTo(A);
		C.copyTo(T);
	}
	if (nchannels == 1)
	{
		int noutheight = IMGSIZE;
		int noutwidth = IMGSIZE;
		float* ImageDst = (float*)outImage;
		float* ImageSrc = (float*)Image;
		float* pData2 = (float*)A.data;
		float* pData1 = (float*)T.data;
		int srcStep = nwidth;
		int dstStep = noutwidth;
		for (int i = 0; i < noutheight; i++)
		{

			for (int j = 0; j < noutwidth; j++)
			{
				float x1 = j * pData2[0] + i * pData2[2] + pData1[0];
				float y1 = j * pData2[1] + i * pData2[3] + pData1[1];
				int x = x1;
				int y = y1;

				if (x < 1 || x >= nwidth - 1 || y < 1 || y >= nheight - 1)
				{
					ImageDst[i*dstStep + j] = value;
				}
				else
				{
					float* p = ImageSrc + y*srcStep;
					float* p1 = ImageSrc + (y + 1)*srcStep;
					float f1x = p[x] * (1 - (x1 - x)) + p[(x + 1)] * (x1 - x);
					float f2x = p1[x] * (1 - (x1 - x)) + p1[(x + 1)] * (x1 - x);
					ImageDst[i*dstStep + j] = f1x*(1 - (y1 - y)) + f2x*((y1 - y));

				}

			}
		}

	}
	else
	{
		int noutheight = IMGSIZE;
		int noutwidth = IMGSIZE;
		
		for (int n = 0; n < nchannels;n++)
		{
			float* ImageDst = (float*)outImage + n*noutwidth*noutheight;
			float* ImageSrc = (float*)Image + n*nheight*nwidth;
			float* pData2 = (float*)A.data;
			float* pData1 = (float*)T.data;
			int srcStep = nwidth;
			int dstStep = noutwidth;
			for (int i = 0; i < noutheight; i++)
			{

				for (int j = 0; j < noutwidth; j++)
				{
					float x1 = j * pData2[0] + i * pData2[2] + pData1[0];
					float y1 = j * pData2[1] + i * pData2[3] + pData1[1];
					int x = x1;
					int y = y1;

					if (x < 1 || x >= nwidth - 1 || y < 1 || y >= nheight - 1)
					{
						ImageDst[i*dstStep + j] = value;
					}
					else
					{
						float* p = ImageSrc + y*srcStep;
						float* p1 = ImageSrc + (y + 1)*srcStep;
						float f1x = p[x] * (1 - (x1 - x)) + p[(x + 1)] * (x1 - x);
						float f2x = p1[x] * (1 - (x1 - x)) + p1[(x + 1)] * (x1 - x);
						ImageDst[i*dstStep + j] = f1x*(1 - (y1 - y)) + f2x*((y1 - y));

					}

				}
			}
		}
	}
	return 0;
}
int AffineLandmark(float* Landmark, const Mat& R, const Mat& C, float* LandmarkOut, bool isInv )

{
	if (Landmark==NULL||LandmarkOut==NULL)
	{
		return -1;
	}
	Mat A, T;
	if (isInv == false)
	{
		A = R.inv();

		T.create(1, 2, CV_32FC1);
		float *pData = (float*)T.data;
		float *pData1 = (float*)C.data;
		float* pData2 = (float*)A.data;
		pData[0] = (pData1[0] * pData2[0] + pData1[1] * pData2[2])*(-1.0);
		pData[1] = (pData1[0] * pData2[1] + pData1[1] * pData2[3])*(-1.0);
	}
	else
	{
		R.copyTo(A);
		C.copyTo(T);
	}
	float* pData = LandmarkOut;
	float* pData1 = Landmark;
	float* a = (float*)A.data;
	float* b = (float*)T.data;
	for (int i = 0; i < LANDMARK; i++)
	{
		pData[0] = pData1[0] * a[0] + pData1[1] * a[2] + b[0];
		pData[1] = pData1[0] * a[1] + pData1[1] * a[3] + b[1];
		pData += 2;
		pData1 += 2;
	}
	return 0;
}

int GetHeatMap(float* Landmark, int HalfSize, float* HeatMap)
{
	
	float* pData = Landmark;
	float* pDataOut = (float*)HeatMap;
	memset(pDataOut, 0, sizeof(float)*IMGSIZE*IMGSIZE);
	for (int i = 0; i < LANDMARK;i++)
	{
		int x = pData[0]+0.5;
		int y = pData[1]+0.5;
		int startx = max(0, x - HalfSize);
		int endx = min(x + HalfSize, IMGSIZE - 1);
		int starty = max(0, y - HalfSize);
		int endy = min(y + HalfSize, IMGSIZE - 1);
		for (int m = starty; m <= endy;m++)
		{
			for (int n = startx; n <= endx; n++)
			{
				int Index = m*IMGSIZE + n;
				float  dis = (m - y)*(m - y) + (n - x)*(n - x);
				float  normdis = sqrtf(dis);
				float hdata = 1.0 / (1 + normdis);
				if (hdata > pDataOut[Index])
				{
					pDataOut[Index] = hdata;
				}
			}
		}
		pData += 2;

	}
	return 0;
}
