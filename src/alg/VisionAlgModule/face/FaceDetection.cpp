#include "FaceDetection.h"
#include "log4cxx/Loging.h"
#include "boost/BoostFun.h"
#include "SubTopic.h"
#include "endecode/Base64.h"
#include "comm/CommFun.h"
#include "CommData.h"

using namespace std;
using namespace cv;
using namespace common_cmmobj;
using namespace common_template;
using namespace Vision_FaceAlg;

const int img_channels = 3;
const int input_size = 1;
const int output_size = 2;
const float fThreshold = 0.9;

FaceDetection::FaceDetection()
{
	SetThreshold(fThreshold);
}

FaceDetection::~FaceDetection()
{
	Release();
}

bool FaceDetection::Init(const Json& algCfg)
{
	//保存算法模型
	string&& strAlgMapKey = string(algCfg[strAttriType]) + string(algCfg[strAttriIdx]);
	auto ite = m_mapAlgModel.find(strAlgMapKey);
	if(ite == m_mapAlgModel.end())
	{
		rknn_context tx;
		m_mapAlgModel.insert(make_pair<string,rknn_context>(string(strAlgMapKey),move(tx)));
	}
	else
	{
		return true;
	}
	
	// open and load model file
	string strPath = string("./model/") + string(algCfg[strAttriModel][strAttriName]);
	FILE *fp = fopen(strPath.c_str(), "rb");
	if(!fp) 
	{
		return false;
	}

	fseek(fp, 0, SEEK_END);   
	int model_len = ftell(fp);   
	void *model = malloc(model_len);
	fseek(fp, 0, SEEK_SET);   
	if(model_len != fread(model, 1, model_len, fp)) 
	{
		free(model);
		fclose(fp);
		return false;
	}
	fclose(fp);

	//init model
	int ret = rknn_init(&m_mapAlgModel[strAlgMapKey],model,model_len,RKNN_FLAG_PRIOR_MEDIUM);
	if(ret < 0) 
	{
		LOG_INFO("face_alg") << "rknn_init fail\n";
		free(model);
		return false;
    }
	LOG_INFO("face_alg") << string_format("rknn_init successful ret : %d and m_tx : %d\n",ret,m_mapAlgModel[strAlgMapKey]);

	//rknn query
	rknn_tensor_attr outputs_attr[output_size];
	outputs_attr[0].index = 0;
	ret = rknn_query(m_mapAlgModel[strAlgMapKey], RKNN_QUERY_OUTPUT_ATTR, &(outputs_attr[0]), sizeof(outputs_attr[0]));
	if (ret < 0)
	{
		LOG_ERROR("face_alg") << string_format("rknn_query fail! ret ： %d\n", ret);
		free(model);
		return false;
	}

	LOG_INFO("face_alg") << string_format("rknn_query successful ret : %d\n",ret);

	return true;
}

void FaceDetection::Run(const string& strIp,const string& strCamCode,const cv::Mat& srcImg)
{	
	//def input
	rknn_input inputs[input_size];
	inputs[0].index = 0;
	inputs[0].size = srcImg.rows * srcImg.cols * img_channels;
	inputs[0].pass_through = false;         
	inputs[0].type = RKNN_TENSOR_UINT8;
	inputs[0].fmt = RKNN_TENSOR_NHWC;
	inputs[0].buf = srcImg.data;

	for(auto val : m_mapAlgModel)
	{
		//img input
		int ret = rknn_inputs_set(val.second, 1, inputs);
		if (ret < 0) 
		{
			LOG_ERROR("face_alg") << string_format("rknn_inputs_set fail! ret=%d\n", ret);
			return;
		}
		
		//rknn run
		ret = rknn_run(val.second, nullptr);
		if (ret < 0) 
		{
			LOG_ERROR("face_alg") << string_format("rknn_run fail! ret=%d\n", ret);
			return;
		}

		//rknn outputs
		rknn_output outputs[output_size];
		for(auto& out : outputs)
		{
			out.want_float = true;
			out.is_prealloc = false;
		}
		ret = rknn_outputs_get(val.second, output_size, outputs, NULL);  //4表示输出的个数
		if (ret < 0)
		{
			LOG_ERROR("face_alg") << string_format("rknn_outputs_get fail! ret=%d\n", ret);
			return;
		}

		//output transform

		//release resource
		rknn_outputs_release(val.second, output_size,outputs);
	}
}

void FaceDetection::OutputTrans(rknn_output outputs[],const Mat& srcImg,const Mat& dstImg,const int& s32Width,const int& s32Height,bool bResize)
{
	// Post Process
	float *loc_data = (float*)(outputs[0].buf);
	int loc_data_length = outputs[0].size / 4;

	float *conf_data = (float*)(outputs[1].buf);
	int conf_data_length = outputs[1].size / 4;
	
	vector<vector<float> > faceinfos;
	faceinfos.clear();

	DetectOutput detectoutput(conf_data_length / 2, dstImg.cols, dstImg.rows);
	int ret = detectoutput.Output(loc_data, conf_data,&faceinfos, GetThreshold());
	if (0 == ret) 
	{
		vector<FaceRectInfo> face_rects;
		for (int i = 0; i < faceinfos.size(); ++i) 
		{
			FaceRectInfo face_item;
			face_item.bbox.x = faceinfos[i][3];
			face_item.bbox.y = faceinfos[i][4];
			face_item.bbox.width = faceinfos[i][5] - faceinfos[i][3];
			face_item.bbox.height = faceinfos[i][6] - faceinfos[i][4];
			face_item.score = faceinfos[i][2];
			if (face_item.bbox.width == 0 || face_item.bbox.height == 0) 
			{
				continue;
			}

			if (bResize) 
			{
				face_item.bbox.x = int(face_item.bbox.x * (srcImg.cols * 1.0f / s32Width));
				face_item.bbox.y = int(face_item.bbox.y * ( srcImg.rows * 1.0f / s32Height));
				face_item.bbox.width = int(face_item.bbox.width * (srcImg.cols * 1.0f / s32Width));
				face_item.bbox.height = int(face_item.bbox.height * (srcImg.rows * 1.0f / s32Height));
			}

			face_item.score = faceinfos[i][2];
			face_rects.push_back(face_item);
		}

		if(face_rects.size() > 0)
		{
			std::cout << "detect face num : " << face_rects.size() << std::endl;
		}
	}
}

void FaceDetection::Release() 
{
	for(auto& val : m_mapAlgModel)
	{
		rknn_destroy(val.second);
	}
}

int DetectOutput::DecodeBBoxes(const float *t_bboxes, const float *p_bboxes,
  float *b_bboxes) 
{
	float p_bbox_w = 0.0f;
	float p_bbox_h = 0.0f;
	float p_bbox_cx = 0.0f;
	float p_bbox_cy = 0.0f;
	int idx = 0;
	for (int i = 0; i < mbox_num_; ++i) {
	idx = i * 4;
	p_bbox_w = p_bboxes[idx+2] - p_bboxes[idx+0];
	p_bbox_h = p_bboxes[idx+3] - p_bboxes[idx+1];
	p_bbox_cx = (p_bboxes[idx+2] + p_bboxes[idx+0]) / 2.f;
	p_bbox_cy = (p_bboxes[idx+3] + p_bboxes[idx+1]) / 2.f;

	float b_bbox_cx = 0.10000000 * t_bboxes[idx+0] * p_bbox_w + p_bbox_cx;
	float b_bbox_cy = 0.10000000 * t_bboxes[idx+1] * p_bbox_h + p_bbox_cy;
	float b_bbox_w = exp(0.20000000 * t_bboxes[idx+2]) * p_bbox_w;
	float b_bbox_h = exp(0.20000000 * t_bboxes[idx+3]) * p_bbox_h;
	b_bboxes[idx+0] = b_bbox_cx - b_bbox_w / 2.f;
	b_bboxes[idx+1] = b_bbox_cy - b_bbox_h / 2.f;
	b_bboxes[idx+2] = b_bbox_cx + b_bbox_w / 2.f;
	b_bboxes[idx+3] = b_bbox_cy + b_bbox_h / 2.f;

	b_bboxes[idx+0] = b_bboxes[idx+0] < 0.f ? 0.f : b_bboxes[idx+0];
	b_bboxes[idx+1] = b_bboxes[idx+1] < 0.f ? 0.f : b_bboxes[idx+1];
	b_bboxes[idx+2] = b_bboxes[idx+2] > 1.f ? 1.f : b_bboxes[idx+2];
	b_bboxes[idx+3] = b_bboxes[idx+3] > 1.f ? 1.f : b_bboxes[idx+3];
	}
	return 0;
}

bool DetectOutput::SortScorePairDescend(const std::pair<int, float>& pair1,
  const std::pair<int, float>& pair2) 
{
	return pair1.second > pair2.second;
}

int DetectOutput::GetMaxScoreIndex(const float *scores, const float thresh,
  const int top_k, std::vector<std::pair<int, float> >* index_score_vec) 
{
	std::vector<std::pair<int, float> > index_score_tmp;
	for (int i = 0; i < mbox_num_; ++i) 
	{
	if (scores[i] > thresh) 
	{
		index_score_tmp.push_back(std::make_pair(i, scores[i]));

	}
	}

	std::stable_sort(index_score_tmp.begin(), index_score_tmp.end(),
	this->SortScorePairDescend);
	index_score_vec->clear();
	index_score_vec->resize(top_k);
	int top_num = top_k;
	if (top_k > index_score_tmp.size()) {
	top_num = index_score_tmp.size();
	}
	for (int i = 0; i < top_num; ++i) {
	(*index_score_vec)[i].first = index_score_tmp[i].first;
	(*index_score_vec)[i].second = index_score_tmp[i].second;
	}
	return 0;
}

// bbox_inter -> vec(4)
int DetectOutput::IntersectBBox(const float *bbox0, const float *bbox1,
  float *bbox_inter) {
  if (bbox1[0] > bbox0[2] || bbox1[2] < bbox0[0] || bbox1[3] < bbox0[1] ||
    bbox0[3] < bbox1[1]) {
    bbox_inter[0] = 0.f;
    bbox_inter[1] = 0.f;
    bbox_inter[2] = 0.f;
    bbox_inter[3] = 0.f;
  } else {
    bbox_inter[0] = std::max(bbox0[0], bbox1[0]);
    bbox_inter[1] = std::max(bbox0[1], bbox1[1]);
    bbox_inter[2] = std::min(bbox0[2], bbox1[2]);
    bbox_inter[3] = std::min(bbox0[3], bbox1[3]);
  }
  return 0;
}

float DetectOutput::BBoxSize(const float *bbox) {
  if (bbox[0] > bbox[2] || bbox[1] > bbox[3]) {
    return 0.f;
  }
  return (bbox[2] - bbox[0]) * (bbox[3] - bbox[1]);
}

float DetectOutput::JaccardOverlap(const float *bbox0, const float *bbox1) 
{
	float* bbox_inter = new float[4];
	IntersectBBox(bbox0, bbox1, bbox_inter);
	float bbox_inter_w = bbox_inter[2] - bbox_inter[0];
	float bbox_inter_h = bbox_inter[3] - bbox_inter[1];
	delete [] bbox_inter;
	if (bbox_inter_h > 0.f && bbox_inter_w > 0.f) 
	{
	float inter_size = bbox_inter_w * bbox_inter_h;
	return inter_size / (BBoxSize(bbox0) + BBoxSize(bbox1) - inter_size);
	}
	return 0.f;
}

int DetectOutput::ApplyNMSFast(const float *b_bboxes, const float *confs,
  const float conf_thresh, const float nms_thresh, const int top_k,
  std::vector<int> *indices) 
{
	std::vector<std::pair<int, float> > idx_scores;
	GetMaxScoreIndex(confs, conf_thresh, top_k, &idx_scores);
	while (idx_scores.size() != 0) {
		int idx = idx_scores[0].first;
		bool keep = true;
		for (int i = 0; i < indices->size(); ++i) {
		if (keep == true) {
			int kept_idx = (*indices)[i];
			float overlap = JaccardOverlap(b_bboxes + idx*4, b_bboxes + kept_idx*4);
			if (overlap <= nms_thresh) {
			keep = true;
			} else {
			keep = false;
			}
		} else {
			break;
		}
		}
		if (keep == true) {
		indices->push_back(idx);
		}
		idx_scores.erase(idx_scores.begin());
	}
	return 0;
}

int DetectOutput::Output(const float* loc_data, const float* conf_data,
  std::vector<std::vector<float> > *outputs, const float face_thresh) 
  {
	const float *t_bboxes = loc_data;
	float *class1_confs = new float [mbox_num_];
	float *p_bboxes = NULL;
	float *b_bboxes = new float [mbox_num_ * 4];

	for (int i = 0; i < mbox_num_; ++i) {
		class1_confs[i] = conf_data[i*2+1];
	}

	GenAllPriorboxs(&p_bboxes);
	DecodeBBoxes(t_bboxes, p_bboxes, b_bboxes);
	if (p_bboxes != NULL) {
		delete [] p_bboxes;
	}
	float conf_thresh = 0.01;
	float nms_thresh = 0.15;
	int top_k = 100;
	std::vector<int> indices;
	ApplyNMSFast(b_bboxes, class1_confs, conf_thresh, nms_thresh, top_k,
		&indices);
	int det_num = indices.size();
	if (det_num == 0) {
		delete [] b_bboxes;
		delete [] class1_confs;
		return 0;
	}
	std::vector<std::pair<int, float> > idx_scores;
	for (int i = 0; i < det_num; ++i) {
		idx_scores.push_back(std::make_pair(indices[i], class1_confs[indices[i]]));
	}
	std::stable_sort(idx_scores.begin(), idx_scores.end(),
		this->SortScorePairDescend);
	if (det_num > top_k) {
		det_num = top_k;
	}
	for (int i = 0; i < det_num; ++i) {
		int idx = idx_scores[i].first;
		float score = idx_scores[i].second;
		if (score < face_thresh) {
		continue;
		}
		std::vector<float> result;
		result.resize(7);
		result[0] = 0.f;
		result[1] = 1.f;
		result[2] = score;
		result[3] = b_bboxes[idx*4]     * image_w_;
		result[4] = b_bboxes[idx*4 + 1] * image_h_;
		result[5] = b_bboxes[idx*4 + 2] * image_w_;
		result[6] = b_bboxes[idx*4 + 3] * image_h_;
		outputs->push_back(result);
	}
	delete [] b_bboxes;
	delete [] class1_confs;
	return 0;
}

int DetectOutput::GenPriorbox(const int image_w, const int image_h,
  const int feamap_w, const int feamap_h, const std::vector<int>& min_sizes,
  const float step, float** prior_data, int* prior_data_length,
  float** variances_data, int* variance_data_length) 
  {
	float step_w = step;
	float step_h = step;

	int p_bboxs_dim = 0;
	for (int i = 0; i < min_sizes.size(); ++i) 
	{
		if (32 == min_sizes[i]) {
		p_bboxs_dim += feamap_h * feamap_w * 4*4 * 4;
		} else if (64 == min_sizes[i]) {
		p_bboxs_dim += feamap_h * feamap_w * 2*2 * 4;
		} else {
		p_bboxs_dim += feamap_h * feamap_w * 1 * 4;
		}
	}

	*prior_data = new float[p_bboxs_dim];
	*variances_data = new float[p_bboxs_dim];
	*prior_data_length = *variance_data_length = p_bboxs_dim;
	float offset = 0.5;
	int idx = 0;
	for (int h = 0; h < feamap_h; ++h) {
		for (int w = 0; w < feamap_w; ++w) {
		float c_x = (w + offset) * step_w;
		float c_y = (h + offset) * step_h;
		for (int s = 0; s < min_sizes.size(); ++s) {
			float box_w = min_sizes[s];
			float box_h = min_sizes[s];
			if (32 == min_sizes[s]) {
			for (int i = -2; i < 2; ++i) {
				for (int j = -2; j < 2; ++j) {
				(*prior_data)[idx] = (c_x + j*8 - (box_w-1) / 2.f) / image_w;
				(*variances_data)[idx] = 0.1;
				idx++;
				(*prior_data)[idx] = (c_y + i*8 - (box_w-1) / 2.f) / image_h;
				(*variances_data)[idx] = 0.1;
				idx++;
				(*prior_data)[idx] = (c_x + j*8 - (box_w-1) / 2.f) / image_w;
				(*variances_data)[idx] = 0.2;
				idx++;
				(*prior_data)[idx] = (c_y + i*8 - (box_w-1) / 2.f) / image_h;
				(*variances_data)[idx] = 0.2;
				idx++;
				}
			}
			} else if (64 == min_sizes[s]) {
			for (int i = -1; i < 1; ++i) {
				for (int j = -1; j < 1; ++j) {
				(*prior_data)[idx] = (c_x + j*16 - (box_w-1) / 2.f) / image_w;
				(*variances_data)[idx] = 0.1;
				idx++;
				(*prior_data)[idx] = (c_y + i*16 - (box_w-1) / 2.f) / image_h;
				(*variances_data)[idx] = 0.1;
				idx++;
				(*prior_data)[idx] = (c_x + j*16 - (box_w-1) / 2.f) / image_w;
				(*variances_data)[idx] = 0.2;
				idx++;
				(*prior_data)[idx] = (c_y + i*16 - (box_w-1) / 2.f) / image_h;
				(*variances_data)[idx] = 0.2;
				idx++;
				}
			}
			} else {
			(*prior_data)[idx] = (c_x - box_w / 2.f) / image_w;
			(*variances_data)[idx] = 0.1;
			idx++;
			(*prior_data)[idx] = (c_y - box_h / 2.f) / image_h;
			(*variances_data)[idx] = 0.1;
			idx++;
			(*prior_data)[idx] = (c_x + box_w / 2.f) / image_w;
			(*variances_data)[idx] = 0.2;
			idx++;
			(*prior_data)[idx] = (c_y + box_h / 2.f) / image_h;
			(*variances_data)[idx] = 0.2;
			idx++;
			}
		}
		}
	}
	return 0;
}

int DetectOutput::GetFeatureMapSize2Cov(const int input_w, const int input_h,
  const int kernel_s, const int stride_s, const int padding_s,
  int *output_w, int *output_h) 
  {
	*output_w = int((input_w - kernel_s + 2.f * padding_s) / stride_s + 1);
	*output_h = int((input_h - kernel_s + 2.f * padding_s) / stride_s + 1);
	return 0;
}

int DetectOutput::GetFeatureMapSize2Pol(const int input_w, const int input_h,
  const int kernel_s, const int stride_s, const int padding_s,
  int *output_w, int *output_h) 
{
	*output_w = int(ceil((input_w - kernel_s + 2.f * padding_s) / stride_s) + 1);
	*output_h = int(ceil((input_h - kernel_s + 2.f * padding_s) / stride_s) + 1);
	return 0;
}

int DetectOutput::GenAllPriorboxs(float** prior_data) 
{
	// pyramid 1
	int output_w = 0;
	int output_h = 0;
	int input_w = 0;
	int input_h = 0;
	GetFeatureMapSize2Cov(image_w_, image_h_, 7, 4, 3, &output_w, &output_h);
	input_w = output_w;
	input_h = output_h;
	GetFeatureMapSize2Pol(input_w, input_h, 3, 2, 0, &output_w, &output_h);
	input_w = output_w;
	input_h = output_h;;
	GetFeatureMapSize2Cov(input_w, input_h, 5, 2, 2, &output_w, &output_h);
	input_w = output_w;
	input_h = output_h;
	GetFeatureMapSize2Pol(input_w, input_h, 3, 2, 0, &output_w, &output_h);


	std::vector<int> min_size1;
	min_size1.resize(3);
	min_size1[0] = 32;
	min_size1[1] = 64;
	min_size1[2] = 128;
	float step1 = 32.0f;
	float* prior_data1;
	int prior_data_length1;
	float* variances_data1;
	int variance_data_length1;
	GenPriorbox(image_w_, image_h_, output_w, output_h, min_size1, step1,
		&prior_data1, &prior_data_length1, &variances_data1, &variance_data_length1);


	// pyramid 2
	input_w = output_w;
	input_h = output_h;
	GetFeatureMapSize2Cov(input_w, input_h, 3, 2, 1, &output_w, &output_h);

	std::vector<int> min_size2;
	min_size2.resize(1);
	min_size2[0] = 256;
	float step2 = 64.0f;
	float* prior_data2;
	int prior_data_length2;
	float* variances_data2;
	int variance_data_length2;
	GenPriorbox(image_w_, image_h_, output_w, output_h, min_size2, step2,
		&prior_data2, &prior_data_length2, &variances_data2, &variance_data_length2);

	// pyramid 3
	input_w = output_w;
	input_h = output_h;
	GetFeatureMapSize2Cov(input_w, input_h, 3, 2, 1, &output_w, &output_h);

	std::vector<int> min_size3;
	min_size3.resize(1);
	min_size3[0] = 512;
	float step3 = 128.0f;
	float* prior_data3;
	int prior_data_length3;
	float* variances_data3;
	int variance_data_length3;
	GenPriorbox(image_w_, image_h_, output_w, output_h, min_size3, step3,
		&prior_data3, &prior_data_length3, &variances_data3, &variance_data_length3);


	int prior_data_length = (prior_data_length1 +
		prior_data_length2 +
		prior_data_length3);

	if (prior_data_length != mbox_num_ * 4) 
	{
		return -3;
	}
	*prior_data = new float [prior_data_length];
	float* p_data = *prior_data;
	memset(p_data, 0, prior_data_length * sizeof(float));
	memcpy(p_data, prior_data1, prior_data_length1 * sizeof(float));
	p_data += prior_data_length1;
	memcpy(p_data, prior_data2, prior_data_length2 * sizeof(float));
	p_data += prior_data_length2;
	memcpy(p_data, prior_data3, prior_data_length3 * sizeof(float));
	delete [] prior_data1;
	delete [] prior_data2;
	delete [] prior_data3;
	delete [] variances_data1;
	delete [] variances_data2;
	delete [] variances_data3;
	return 0;
}
