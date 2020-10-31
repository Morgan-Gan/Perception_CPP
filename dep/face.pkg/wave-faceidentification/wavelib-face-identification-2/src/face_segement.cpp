#include "face_segement.h"

void rotate_img(cv::Mat& src, cv::Mat& dst, cv::Mat pts, cv::Mat &ptsdst, double theta, cv::Point2f pt)
{
	double angle = theta * 180 / 3.141592654;
	cv::Mat rotate_mat = cv::getRotationMatrix2D(pt, angle, 1.0);
	int row_dst = src.rows * cos(abs(theta)) + src.cols * sin(abs(theta));
	int col_dst = src.rows * sin(abs(theta)) + src.cols * cos(abs(theta));
	cv::Mat roteMat;
	roteMat.create(rotate_mat.rows, rotate_mat.cols, CV_32FC1);
	float* pData = (float*)roteMat.data;
	double* pData1 = (double*)rotate_mat.data;
	for (int m = 0; m < roteMat.rows; m++)
	{
		pData = (float*)(roteMat.data + m*roteMat.step[0]);
		pData1 = (double*)(rotate_mat.data + m*rotate_mat.step[0]);
		for (int n = 0; n < roteMat.cols; n++)
		{
			pData[n] = pData1[n];
		}
	}
	ptsdst = roteMat*pts;
	cv::warpAffine(src, dst, rotate_mat, cv::Size(col_dst, row_dst));
}


int CropImgByFace(cv::Mat img, cv::Mat& crop_img, cv::Rect rect, cv::Point2f pts[68], int pointNum, int isIDCardPthoto)
{
	if (pointNum != 68||img.data== NULL)
	{
		return -1;
	}
	int min_x=1000000000;
	int min_y=1000000000;
	int max_x= 0;
	int max_y = 0;
	for(int i = 0; i< 68; i++)
	{
		if(pts[i].x <min_x) min_x = pts[i].x;
		if(pts[i].x >max_x) max_x = pts[i].x;
		if(pts[i].y <min_y) min_y = pts[i].y;
		if(pts[i].y >max_y) max_y = pts[i].y;
	}
	min_x = cv::min(min_x,rect.x);
	min_y = cv::min(min_y,rect.y);
	max_x = cv::max(max_x,rect.x+ rect.width-1);
	max_y = cv::max(max_y,rect.y+ rect.height-1);
	cv::Rect face;
	face.x = min_x;
	face.y = min_y;
	face.width = max_x -min_x +1;
	face.height = max_y - min_y +1;
	if(face.width<=40||face.height<=40||face.x<0||face.x>img.cols-1||
		face.width>img.cols||face.height>img.rows||face.y<0||face.y>img.rows-1)
	{
		return -1;
	}
	cv::Rect faceRect;
	faceRect.x = fmax(face.x - 0.25*face.width, 0);
	faceRect.width = 1.5*face.width;
	if (faceRect.x + 1.5*face.width > img.cols - 1)
	{
		faceRect.width = (img.cols - faceRect.x - 1);
	}
	faceRect.y = fmax(face.y - 0.25*face.height, 0);
	faceRect.height = 1.5*face.height;
	if (faceRect.y + 1.5*face.height > img.rows - 1)
	{
		faceRect.height = (img.rows - faceRect.y - 1);
	}
	cv::Mat crop_faceimg = img(faceRect);
	cv::Point2f le, re, nose, lm, rm;
	le.x = (pts[36].x + pts[37].x + pts[38].x + pts[39].x + pts[40].x + pts[41].x) / 6.0 - faceRect.x;
	le.y = (pts[36].y + pts[37].y + pts[38].y + pts[39].y + pts[40].y + pts[41].y) / 6.0 - faceRect.y;
	re.x = (pts[42].x + pts[43].x + pts[44].x + pts[45].x + pts[46].x + pts[47].x) / 6.0 - faceRect.x;
	re.y = (pts[42].y + pts[43].y + pts[44].y + pts[45].y + pts[46].y + pts[47].y) / 6.0 - faceRect.y;
	nose.x = pts[30].x - faceRect.x;
	nose.y = pts[30].y - faceRect.y;
	lm.x = pts[48].x - faceRect.x;
	lm.y = pts[48].y - faceRect.y;
	rm.x = pts[54].x - faceRect.x;
	rm.y = pts[54].y - faceRect.y;
	cv::Point2f pt(nose.x, nose.y);
	float centor_x = (le.x + re.x + nose.x + lm.x + rm.x) / 5;
	float centor_y = (le.y + re.y + nose.y + lm.y + rm.y) / 5;
	if (centor_x <= 0 || centor_x > crop_faceimg.cols - 1 || centor_y <= 0 || centor_y > crop_faceimg.rows - 1)
	{
		return -1;
	}
	cv::Point2f pt_centor(centor_x, centor_y);
	double theta = atan((re.y - le.y) / (re.x - le.x));
	cv::Mat img_rotated;
	cv::Mat ptsMat = cv::Mat::ones(3, 68, CV_32FC1);
	float* pData = (float*)(ptsMat.data + ptsMat.step[0]);
	float* pData1 = (float*)(ptsMat.data);
	for (int k = 0; k < 68; k++)
	{
		pData[k] = pts[k].y - faceRect.y;
		pData1[k] = pts[k].x - faceRect.x;
	}
	cv::Mat ptsDst;
	rotate_img(crop_faceimg, img_rotated, ptsMat, ptsDst, theta, pt_centor);
	double dis_eye = sqrt((re.y - le.y)*(re.y - le.y) + (re.x - le.x)*(re.x - le.x));
	double dis_em = sqrt(((re.y + le.y) / 2 - (rm.y + lm.y) / 2)*((re.y + le.y) / 2 - (rm.y + lm.y) / 2) + ((re.x + le.x) / 2 - (rm.x + lm.x) / 2)*((re.x + le.x) / 2 - (rm.x + lm.x) / 2));
	double dis = fmax(dis_eye, dis_em);
	int x_min = fmax(0, floor(centor_x - dis*1.3));
	int x_max = fmin(img_rotated.cols - 1, ceil(centor_x + dis*1.3));
	int y_min = fmax(0, floor(centor_y - dis*1.3));
	int y_max = fmin(img_rotated.rows - 1, ceil(centor_y + dis*1.3));
	crop_img = img_rotated(cv::Rect(x_min, y_min, x_max - x_min, y_max - y_min));
	
	return 0;
}
