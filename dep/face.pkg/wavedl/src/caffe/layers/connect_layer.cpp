#include <functional>
#include <utility>
#include <vector>
#include "caffe/DAN.h"
#include "caffe/layers/connect_layer.hpp"
#include "caffe/util/math_functions.hpp"

namespace caffe {

template <typename Dtype>
void ConnectLayer<Dtype>::LayerSetUp(
  const vector<Blob<Dtype>*>& bottom, const vector<Blob<Dtype>*>& top) {
	CHECK_EQ(bottom.size(), 3) << "ConnectLayer takes a single blob as input.";
	CHECK_EQ(top.size(), 3) << "ConnectLayer takes a single blob as output.";
	CHECK_EQ(bottom[2]->num(), bottom[0]->num()) << "ConnectLayer takes a single blob as output.";// 2 pre_landmakr  1 gt_landmark 
	CHECK_EQ(bottom[0]->num(), bottom[1]->num()) << "ConnectLayer takes a single blob as output.";
	CHECK_EQ(bottom[1]->count(), bottom[2]->count()) << "ConnectLayer takes a single blob as output.";
	num_ = bottom[0]->num();
	channels_ = bottom[0]->channels();
	height_ = bottom[0]->height();
	width_ = bottom[0]->width();
	landmarks_ = bottom[1]->count() / num_;
	
}

template <typename Dtype>
void ConnectLayer<Dtype>::Reshape(
	const vector<Blob<Dtype>*>& bottom, const vector<Blob<Dtype>*>& top) {
	top[0]->Reshape(num_,channels_,height_,width_);
	top[1]->Reshape(num_, 1, height_, width_);
	vector<int> label_shape;
	label_shape.push_back(num_);
	label_shape.push_back(landmarks_);
	top[2]->Reshape(label_shape);
	label_shape[0] = 1;
	pre_landmarks_.Reshape(label_shape);
	trans_gt_landmarks_.Reshape(label_shape);
	trans_pre_landmarks_.Reshape(label_shape);
	label_shape.clear();
}
template <typename Dtype>
void ConnectLayer<Dtype>::Forward_cpu(
	const vector<Blob<Dtype>*>& bottom, const vector<Blob<Dtype>*>& top) {
	const Dtype meanshape[] = { 27.07073828, 40.82285702, 27.23700584, 48.69697332, 28.13616493
		, 56.61469355, 29.71880067, 64.42449433, 32.53966357, 71.62860404
		, 36.94256555, 77.96991301, 42.54833787, 83.27223551, 49.0898234
		, 87.27404899, 56.55019928, 88.44473259, 63.95435631, 87.18051466
		, 70.46482926, 83.15999686, 76.05522789, 77.84549007, 80.37374406
		, 71.38877838, 83.12297152, 64.04028684, 84.65294974, 56.22152822
		, 85.42889203, 48.24360896, 85.53304006, 40.29287418, 31.97296755
		, 34.12401035, 35.52475334, 30.59829692, 40.68888172, 29.50112247
		, 46.03643371, 30.26772612, 50.91698125, 32.2223827, 60.87000138
		, 31.92663397, 65.94151562, 29.89449003, 71.33842456, 29.18361271
		, 76.53186259, 30.33783098, 80.06956662, 33.87597444, 55.95697748
		, 39.00673694, 55.95489265, 44.07291718, 55.95384538, 49.00515766
		, 55.97424838, 54.12163136, 49.74302324, 58.53163751, 52.77265678
		, 59.53911397, 56.06082163, 60.27281704, 59.32781592, 59.48990353
		, 62.32249866, 58.5382276, 37.53495382, 40.52156226, 40.81390227
		, 38.80547415, 44.63455685, 38.8332459, 48.07630179, 40.96052256
		, 44.54131404, 41.80399039, 40.73827957, 41.84358277, 63.80350807
		, 40.8324307, 67.28417686, 38.5902594, 71.1085882, 38.57729696
		, 74.39712714, 40.21324876, 71.27499455, 41.51062622, 67.53309373
		, 41.6027777, 45.20366465, 69.13995397, 49.14968056, 66.84226068
		, 53.22458641, 65.61720842, 56.09967159, 66.39386474, 59.12968227
		, 65.61371934, 63.29210515, 66.8192887, 67.20418052, 69.01108312
		, 63.44313283, 72.53049169, 59.50923752, 74.20910401, 56.15705774
		, 74.55059435, 52.95575727, 74.25455012, 49.0368352, 72.60384779
		, 46.88947732, 69.18414025, 53.16874556, 68.55473865, 56.14203285
		, 68.83368789, 59.25473132, 68.51232491, 65.52511074, 69.08928986
		, 59.28956968, 70.18609756, 56.100913, 70.55705049, 53.1093319
		, 70.23984673 };
	for (int m = 0; m < num_;m++)
	{
		/*if (m%4 !=0|| this->phase_ == TEST)
		{
			caffe_add(landmarks_, bottom[2]->cpu_data() + bottom[2]->offset(m), meanshape, pre_landmarks_.mutable_cpu_data());
		}
		else
		{
			float* pData = (float*)pre_landmarks_.mutable_cpu_data();
			for (int k = 0; k < landmarks_; k++)
			{
				int step = rand() % 3 - 1;
				*pData += step;
				pData++;
			}
			
		}*/
		
		if (m%3< 2||this->phase_== TEST)
		{
			if (m % 4 != 0 || this->phase_ == TEST)
			{
				caffe_add(landmarks_, bottom[2]->cpu_data() + bottom[2]->offset(m), meanshape, pre_landmarks_.mutable_cpu_data());
			}
			else
			{
				float* pData = (float*)pre_landmarks_.mutable_cpu_data();
				for (int k = 0; k < landmarks_; k++)
				{
					int step = rand() % 3 - 1;
					*pData += step;
					pData++;
				}

			}
			//caffe_add(landmarks_, bottom[2]->cpu_data() + bottom[2]->offset(m), meanshape, pre_landmarks_.mutable_cpu_data());
			Mat T, C;
			GetAffineParam((float*)pre_landmarks_.mutable_cpu_data(), (float*)meanshape, T, C);
			AffineLandmark((float*)bottom[1]->mutable_cpu_data() + bottom[1]->offset(m), T, C, (float*)trans_gt_landmarks_.mutable_cpu_data(), true);
			AffinefloatImage((float*)bottom[0]->mutable_cpu_data() + bottom[0]->offset(m), height_, width_, channels_,
				T, C, (float*)top[0]->mutable_cpu_data() + top[0]->offset(m), -0.5,true);
			AffineLandmark((float*)pre_landmarks_.mutable_cpu_data(), T, C, (float*)trans_pre_landmarks_.mutable_cpu_data(), true);
			GetHeatMap((float*)trans_pre_landmarks_.mutable_cpu_data(), 8, (float*)top[1]->mutable_cpu_data() + top[1]->offset(m));
			caffe_sub(landmarks_, (float*)trans_gt_landmarks_.cpu_data(), (float*)trans_pre_landmarks_.mutable_cpu_data(),
				(float*)top[2]->mutable_cpu_data() + top[2]->offset(m));
		}
		else
		{
			caffe_add(landmarks_, bottom[2]->cpu_data() + bottom[2]->offset(m), meanshape, pre_landmarks_.mutable_cpu_data());
			Mat T, C;
			GetAffineParam((float*)pre_landmarks_.mutable_cpu_data(), (float*)meanshape, T, C);
			AffineLandmark((float*)bottom[1]->mutable_cpu_data() + bottom[1]->offset(m), T, C, (float*)trans_gt_landmarks_.mutable_cpu_data(), false);
			float min_value = 1000;
			float max_value = 0.0;
			float* pData = (float*)trans_gt_landmarks_.mutable_cpu_data();
			for (int k = 0; k < landmarks_; k++)
			{
				if (*pData < min_value)
				{
					min_value = *pData;
				}
				if (*pData > max_value)
				{
					max_value = *pData;
				}
				pData++;
			}
			bool flag = false;
			if (min_value <= 0 || max_value > height_ - 1)
			{
				flag = true;
				AffineLandmark((float*)bottom[1]->mutable_cpu_data() + bottom[1]->offset(m), T, C, (float*)trans_gt_landmarks_.mutable_cpu_data(), flag);
			}
			AffinefloatImage((float*)bottom[0]->mutable_cpu_data() + bottom[0]->offset(m), height_, width_, channels_,
				T, C, (float*)top[0]->mutable_cpu_data() + top[0]->offset(m), -0.5, flag);
			AffineLandmark((float*)pre_landmarks_.mutable_cpu_data(), T, C, (float*)trans_pre_landmarks_.mutable_cpu_data(), flag);
			GetHeatMap((float*)trans_pre_landmarks_.mutable_cpu_data(), 8, (float*)top[1]->mutable_cpu_data() + top[1]->offset(m));
			caffe_sub(landmarks_, (float*)trans_gt_landmarks_.cpu_data(), (float*)trans_pre_landmarks_.mutable_cpu_data(),
				(float*)top[2]->mutable_cpu_data() + top[2]->offset(m));
		}
		
		

	}
	caffe_set(bottom[0]->count(), Dtype(0.0), bottom[0]->mutable_cpu_diff());
	caffe_set(bottom[1]->count(), Dtype(0.0), bottom[1]->mutable_cpu_diff());
	caffe_set(bottom[2]->count(), Dtype(0.0), bottom[2]->mutable_cpu_diff());
	caffe_set(top[0]->count(), Dtype(0.0), top[0]->mutable_cpu_diff());
	caffe_set(top[1]->count(), Dtype(0.0), top[1]->mutable_cpu_diff());
	caffe_set(top[2]->count(), Dtype(0.0), top[2]->mutable_cpu_diff());
	
}
template <typename Dtype>
void ConnectLayer<Dtype>::Backward_cpu(const vector<Blob<Dtype>*>& top,
	const vector<bool>& propagate_down, const vector<Blob<Dtype>*>& bottom) {

	
}

INSTANTIATE_CLASS(ConnectLayer);
REGISTER_LAYER_CLASS(Connect);
}  // namespace caffe
