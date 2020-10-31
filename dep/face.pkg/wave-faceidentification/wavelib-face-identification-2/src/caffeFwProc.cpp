// Copyright 2014 BVLC and contributors.
//
// This is a simple script that allows one to quickly test a network whose
// structure is specified by text format protocol buffers, and whose parameter
// are loaded from a pre-trained network.
// Usage:
//    test_net net_proto pretrained_net_proto iterations [CPU/GPU]

#include "caffeFwProc.h"
#include <sys/stat.h>  
unsigned long get_model_size(const char *modelname) {  
    unsigned long filesize = -1;      
    struct stat statbuff;  
    if(stat(modelname, &statbuff) < 0){  
        return filesize;  
    }else{  
        filesize = statbuff.st_size;  
    }  
    return filesize;  
} 
void encode_net(const char* modelfile) {
	char filename[512];
	memset(filename,0,512);
	sprintf(filename, "model.cpp");
	FILE* pfOut = fopen(filename, "w+");
	fprintf(pfOut, "char MODEL_1[] = {\n");
	FILE* pfIn = fopen(modelfile, "rb");
	int ret = 0;
	while (!feof(pfIn)){
		char c ;
		ret = fread(&c, 1, 1, pfIn);
		if (ret<=0)
		{
			continue;
		}
		c = ~c;
		fprintf(pfOut,"%d, ", c);
	}
	fprintf(pfOut,"\n0 };");
	fclose(pfIn);
	fclose(pfOut);
}

void decode_net(const char* src, char* dst) {
	int len = strlen(src);
	for (int i = 0; i < len; i++){
		dst[i] = ~src[i];
	}
}
bool is_model_need_decode(unsigned char* ptr) {
	char buf[50];
	memcpy(buf, ptr + 2, 7);
	buf[7] = '\0';
	if (strcmp(buf, "AlexNet") == 0||strcmp(buf,"FaceNet")==0||strcmp(buf,"Hamming")==0)
		return false;
	return true;
}
void decode_model(unsigned char* ptr, int len) {
	int nPair = len / 2;
	for (int i = 0; i < nPair; i++){
		unsigned char tmp;
		tmp = ~ptr[i];
		ptr[i] = ~ptr[len - 1 - i];
		ptr[len - 1 - i] = tmp;
	}
	if (len > 2 * nPair)
		ptr[nPair] = ~ptr[nPair];
}
int ForwardInit(void ** net, const char *model, const char *weight, int bgpu) {
	const char *FLAGS_model = model;
	const char *FLAGS_weights = weight;
	char *FLAGS_iterations = "1";
	int FLAGS_gpu = bgpu;
	CHECK_GT(strlen(FLAGS_model), 0) << "Need a model definition to score.";
	CHECK_GT(strlen(FLAGS_weights), 0) << "Need model weights to score.";

	int Len = strlen(FLAGS_model);
	Net<float> *caffe_net = NULL;
	if (Len < 1000){
		caffe_net = new Net<float>(FLAGS_model, caffe::TEST);
		encode_net(FLAGS_model);
	}	else{
		NetParameter param;
		char* buf = new char[Len+1];
		decode_net(FLAGS_model, buf);
		buf[Len] = '\0';
		bool success = ReadProtoFromText(buf, &param);
		if (!success)
		{
			delete[] buf;
			return NP_ERROR_INIT;
		}
		param.mutable_state()->set_phase(TEST);
		param.mutable_state()->set_level(0);
		//param.set_initial_skip(1);
		caffe_net = new Net<float>(param);
		delete[] buf;
	}

	unsigned long fileSize = get_model_size(FLAGS_weights);
	unsigned char* weights = new unsigned char[fileSize];
	FILE *pf = fopen(FLAGS_weights, "rb");
	fread(weights, 1, fileSize, pf);
	fclose(pf);
	//printf("file size : %d\n",fileSize);
	if (is_model_need_decode(weights)){
		decode_model(weights, fileSize);
	}
	else{
		decode_model(weights, fileSize);
		char filename[512];
		memset(filename,0,512);
		sprintf(filename, "%s.bin", FLAGS_weights);
		FILE* pfOut = fopen(filename, "wb+");
		fwrite(weights, 1, fileSize, pfOut);
		fclose(pfOut);
		decode_model(weights, fileSize);
	}
	NetParameter trained_net_param;
	CHECK(ReadProtoFromBinaryMemory(weights,fileSize, &trained_net_param)) << "Failed to parse NetParameter file";
	delete[] weights;
	caffe_net->CopyTrainedLayersFrom(trained_net_param);
	*net = caffe_net;
	return 0;
}
void MatTransformBlob(const cv::Mat& cv_img, Blob<float>* transformed_blob, vector<float>mean, float scale) {
	const int img_channels = cv_img.channels();
	const int img_height = cv_img.rows;
	const int img_width = cv_img.cols;
	const int channels = transformed_blob->channels();
	const int height = transformed_blob->height();
	const int width = transformed_blob->width();
	const int num = transformed_blob->num();
	CHECK_EQ(channels, img_channels);
	CHECK_EQ(channels, mean.size());
	CHECK_LE(height, img_height);
	CHECK_LE(width, img_width);
	CHECK_GE(num, 1);
	CHECK(cv_img.depth() == CV_8U) << "Image data type must be unsigned byte";
	float* transformed_data = transformed_blob->mutable_cpu_data();
	int top_index;
	for (int h = 0; h < img_height; ++h) {
		const uchar* ptr = cv_img.ptr<uchar>(h);
		int img_index = 0;
		for (int w = 0; w < img_width; ++w) {
			for (int c = 0; c < img_channels; ++c) {
				top_index = (c * img_height + h) * img_width + w;
				float pixel;
				// int top_index = (c * height + h) * width + w;
				if (c==0) {
					pixel = (static_cast<float>(ptr[img_index++]) - mean[0]);
				} else if(c==1) {
					pixel = (static_cast<float>(ptr[img_index++]) - mean[1]);
				} else if (c==2) {
					pixel = (static_cast<float>(ptr[img_index++]) - mean[2]);
				}	else {
					pixel = (static_cast<float>(ptr[img_index++]) - mean[2]);
				}
				transformed_data[top_index] = pixel*scale;
			}
		}
	}
}
void MatArrayTransformBlob(const vector<cv::Mat> & mat_vector,Blob<float>* transformed_blob,vector<float>mean,float scale) {
	const int mat_num = mat_vector.size();
	const int num = transformed_blob->num();
	const int channels = transformed_blob->channels();
	const int height = transformed_blob->height();
	const int width = transformed_blob->width();
	CHECK_GT(mat_num, 0) << "There is no MAT to add";
	CHECK_EQ(mat_num, num) <<
		"The size of mat_vector must be equals to transformed_blob->num()";
	Blob<float> uni_blob(1, channels, height, width);
	for (int item_id = 0; item_id < mat_num; ++item_id) {
		int offset = transformed_blob->offset(item_id);
		uni_blob.set_cpu_data(transformed_blob->mutable_cpu_data() + offset);
		MatTransformBlob(mat_vector[item_id], &uni_blob, mean, scale);
	}
}

int ForwardProcess(Net<float> *caffe_net, Net<float> *caffe_net1, WV_FACE_ParamIn *inPara, StructOutParam *outPara)
{
	try
	{
		if (inPara->nImg.nChannel != 3 || caffe_net == NULL)
		{
			return NP_ERROR_PARAM;
		}
		//StructNSBDHandle * procHandel = (StructNSBDHandle*)handle;
		cv::Mat  cv_img_origin(cv::Size(inPara->nImg.width, inPara->nImg.height), CV_8UC3, inPara->nImg.pData, inPara->nImg.nWidthStep);
		cv::Mat cropImg;
		if (inPara->nFacePoint != NULL)
		{
			cv::Rect face;
			int left = inPara->nFacePoint->pFaceRect.left;
			face.x = std::max(left, 0);
			int top = inPara->nFacePoint->pFaceRect.top;
			face.y = std::max(top, 0);
			int right = inPara->nFacePoint->pFaceRect.right;
			face.width = std::min(right, cv_img_origin.cols - 1) - inPara->nFacePoint->pFaceRect.left + 1;
			if (face.x + face.width > cv_img_origin.cols - 2)
			{
				face.width = cv_img_origin.cols - 2 - face.x;
			}
			int bottom = inPara->nFacePoint->pFaceRect.bottom;
			face.height = std::min(bottom, cv_img_origin.rows - 1) - inPara->nFacePoint->pFaceRect.top + 1;
			if (face.y + face.height > cv_img_origin.rows - 2)
			{
				face.height = cv_img_origin.rows - 2 - face.y;
			}
			if (inPara->nFacePoint->nFacePointNum != 68 || face.width <= 40 || face.height <= 40 || face.x <0 || face.x > cv_img_origin.cols - 1 ||
				face.width > cv_img_origin.cols || face.height > cv_img_origin.rows || face.y <0 || face.y > cv_img_origin.rows - 1)
			{
				
				cv_img_origin.copyTo(cropImg);
			}
			else
			{
				cv::Point2f pts[68];
				for (int k = 0; k < inPara->nFacePoint->nFacePointNum&&k < 68; k++)
				{
					pts[k].x = inPara->nFacePoint->pPoints[k].x;
					pts[k].y = inPara->nFacePoint->pPoints[k].y;
				}
				int ret = CropImgByFace(cv_img_origin, cropImg, face, pts, 68, 0); 
				if (ret)
				{
					cropImg = cv_img_origin(face);
				}
				
			}

		}
		else
		{
			cv_img_origin.copyTo(cropImg);
			
		}
		Blob<float>* input = caffe_net->blob_by_name("data").get();
		Blob<float>* input1 = caffe_net1->blob_by_name("data").get();
		int rwidth = input->width();
		int rheight = input->height();
		int rchannels = input->channels();
		Size dsize = Size(rwidth, rheight);
		vector<Mat> matarray;
		vector<Mat> matarray1;
		if (inPara->nImg.nType == WV_BGR)
		{
				Mat cv_img, tran_img;
				resize(cropImg, cv_img, dsize, 0, 0, CV_INTER_LINEAR);
				//imwrite("test.jpg",cv_img);
				matarray.push_back(cv_img);
				matarray1.push_back(cv_img);
				
		}
		else
		{
				Mat cv_img, tran_img;
				resize(cropImg, tran_img, dsize, 0, 0, CV_INTER_LINEAR);
				cvtColor(tran_img, cv_img, CV_BGR2RGB);
				matarray.push_back(cv_img);
				matarray1.push_back(cv_img);
		}
		vector<float>mean;
		mean.push_back(127.5);
		mean.push_back(127.5);
		mean.push_back(127.5);
		vector<float>mean1;
		mean1.push_back(104);
		mean1.push_back(117);
		mean1.push_back(123);
		if (matarray.size() == 1&&matarray1.size()==1)
		{
			input->Reshape(matarray.size(), rchannels, rheight, rwidth);
			input1->Reshape(matarray1.size(), rchannels, rheight, rwidth);
			MatArrayTransformBlob(matarray, input, mean, 0.0078125);
			MatArrayTransformBlob(matarray1, input1,mean1,1.0);
			matarray.clear();
			matarray1.clear();
			caffe_net->ForwardPrefilled();
			caffe_net1->ForwardPrefilled();
			Blob<float>* features = caffe_net->blob_by_name("embedding/normalize").get();//
			Blob<float>* features1 = caffe_net1->blob_by_name("embedding/normalize").get();//
			outPara->pdata = features->mutable_cpu_data();
			outPara->dim = features->count();
			outPara->pdata1 = features1->mutable_cpu_data();
			outPara->dim1 = features1->count();
		}
		if (!matarray.empty())
		{
			matarray.clear();
		}
		if (!matarray1.empty())
		{
			matarray1.clear();
		}
		if (!mean1.empty())
		{
			mean1.clear();
		}
		if (!mean.empty())
		{
			mean.clear();
		}
		return 0;
	}
	catch (...)
	{
		
		return -1;
	}
}

int ForwardUinit()
{	
	return 0;
}
