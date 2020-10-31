#include <vector>

#include "caffe/layers/zk_conv.hpp"
#include "device_launch_parameters.h" 

#include <math_functions.h>  // CUDA's, not caffe's, for fabs, signbit
#include <thrust/device_vector.h>
#include <thrust/functional.h>  // thrust::plus
#include <thrust/reduce.h>

namespace caffe {
	template <typename Dtype>
	__global__ void backward_bias_gpuwithgroup(const int Batchsize, 
		const Dtype *top_diff, const int top_rows, const int top_cols, const int top_depth, 
		Dtype *bias_diff,const int bias_Num){
		int thread_id = (blockIdx.x + blockIdx.y*gridDim.x) * blockDim.x + threadIdx.x;
		if (thread_id >= bias_Num) return;
		const int top_diff_offset = thread_id*top_rows*top_cols;
		Dtype gradient = 0;
		for (int n = 0; n < Batchsize; n++)	{
			const int top_offset = n*top_depth*top_cols*top_rows;
			for (int i = 0; i < top_rows*top_cols; i+=4){
				gradient += top_diff[top_offset + top_diff_offset + i];
				gradient += top_diff[top_offset + top_diff_offset + i+1];
				gradient += top_diff[top_offset + top_diff_offset + i+2];
				gradient += top_diff[top_offset + top_diff_offset + i+3];
			}			
		}		
		bias_diff[thread_id] = gradient;
	}

	/*��bottom�ݶȵķ��򴫲�Ϊtop_diff��weight��ת180�Ⱥ���о��
	* �����̼߳���ÿһ��bottom���ݶ�
	*/
	template <typename Dtype>
	__global__ void backward_Input_gpuwithgroup(const Dtype *top_diff, const int top_rows, const int top_cols, const int top_depth, const int top_group,
		const Dtype *weight, const int weight_rows, const int weight_cols, const int weight_depth,
		const int stride, const int pad_rows, const int pad_cols,
		const int bottom_rows, const int bottom_cols, const int bottom_depth, const int bottom_group, Dtype *bottom_diff, const int bottom_Number){

		int thread_id = (blockIdx.x + blockIdx.y*gridDim.x) * blockDim.x + threadIdx.x;
		if (thread_id >= bottom_Number) return;

		const int B_col = thread_id%bottom_cols;
		const int B_row = (thread_id / bottom_cols) % bottom_rows;
		const int B_chanel = (thread_id / bottom_cols / bottom_rows) % bottom_depth;
		const int B_batch = thread_id / bottom_cols / bottom_rows / bottom_depth;

		/* bottom_groupΪ����ÿ����������ͨ����
		*top_groupΪ���ÿ����������ͨ����
		*���㷽ʽ�����ȸ���B_chanel�������ǰͨ�����ڵڼ���Nth_group=B_chanel/bottom_group;
		*����Nth_group��������top_diff��ƫ��top_offset=Nth_group*top_group*top_cols*top_rows;
		*Ȩ��weightƫ��kernel_offset=OD*in_group*kernel_cols*kernel_rows;
		*/

		int Nth_group = B_chanel / bottom_group;
		int Nth_chanel = B_chanel%bottom_group;

		const Dtype *top_diff_slice = top_diff + B_batch*top_depth*top_rows*top_cols + Nth_group*top_group*top_cols*top_rows;
		const int weight_rotate_offset = Nth_group*top_group*weight_depth*weight_cols*weight_rows + (Nth_chanel + 1)*weight_rows*weight_cols - 1;


		int top_C_start = B_col + pad_cols - weight_cols + 1;
		int top_C_end = top_C_start + weight_cols;
		int top_R_start = B_row + pad_rows - weight_rows + 1;
		int top_R_end = top_R_start + weight_rows;

		Dtype gradient = 0;
		for (int top_Cindex = 0; top_Cindex < top_group; top_Cindex++){
			int top_diff_slice_offset = top_Cindex*top_rows*top_cols;
			int weight_offset = top_Cindex*weight_depth*weight_cols*weight_rows+weight_rotate_offset;
			for (int top_R = top_R_start; top_R < top_R_end; top_R++){
				for (int top_C = top_C_start; top_C < top_C_end; top_C++){
					if (top_R%stride == 0 && top_C%stride == 0){
						int top_r = top_R / stride;
						int top_c = top_C / stride;
						if (top_r >= 0 && top_r < top_rows&&top_c >= 0 && top_c < top_cols){
							int topoffset = top_r*top_cols + top_c;
							//std::cout << "gradient=" << thread_id << "  " << top_diff_slice_offset + topoffset << " * " << weight_offset << endl;
							gradient += *(top_diff_slice + top_diff_slice_offset + topoffset)*(*(weight + weight_offset));
						}
					}
					weight_offset--;
				}
			}
		}
		bottom_diff[thread_id]=gradient;
	}
	


	
	///��Ȩ�ص��ݶ�Ϊ input �� output�ľ��
	//���㷽ʽΪ�����߳�ֱ�Ӽ���Ȩ���ݶȵ�ÿһ��ֵ
	/*groupΪ������ʱ*/
	//numΪbatchsize��
	template <typename Dtype>
	__global__ void backward_weight_gpuwithgroup(const int num, const Dtype *inputdata, const int in_group, const int in_rows, const int in_cols, const int in_depth,
		const Dtype *output_diff, const int out_group, const int out_rows, const int out_cols, const int out_depth,
		const int stride, const int pad_rows, const int pad_cols,
		const int weight_rows, const int weight_cols, const int weight_depth, Dtype *weight_diff, const int weight_Number){


		int thread_id = (blockIdx.x + blockIdx.y*gridDim.x) * blockDim.x + threadIdx.x;
		if (thread_id >= weight_Number) return;

		///Ȩ�ص����з�ʽΪNxMxKxK; NΪ���������MΪ�����ͨ������KΪkernel�ĸ߶ȺͿ��
		const int K_col = thread_id % weight_cols;													//Kernel����
		const int K_row = (thread_id / weight_cols) % weight_rows;									//Kernel����
		const int K_chanel = (thread_id / weight_cols / weight_rows) % weight_depth;				//Kernel��ͨ�����������ͨ����
		const int K_N = thread_id / weight_cols / weight_rows / weight_depth;						//Kernel�ĸ��� �������ͨ����


		/* in_groupΪ����ÿ����������ͨ����
		*out_groupΪ���ÿ����������ͨ����
		*���㷽ʽ�����ȸ���OD�������ǰͨ�����ڵڼ���Nth_group=OD/out_group;
		*����Nth_group��������inputdata��ƫ��in_offset=Nth_group*in_group*in_cols*in_rows;
		*Ȩ��weightƫ��kernel_offset=OD*in_group*kernel_cols*kernel_rows;
		*/
		const int Nth_group = K_N / out_group;

		const int input_offset = in_rows*in_cols*in_depth;
		const int out_offset = out_rows*out_cols*out_depth;
		//const int input_row_start = K_row*stride - pad_rows;
		Dtype gradient = 0;
		for (int n = 0; n < num; n++){
			const Dtype* const inputdata_slice = inputdata + n*input_offset + K_chanel*in_cols*in_rows + Nth_group*in_group*in_cols*in_rows;					//nΪbatsize�еĵڼ��� ����ƫ��
			const Dtype* const outputdata_slice = output_diff + n*out_offset + K_N*out_cols*out_rows;					//���ƫ��
			for (int out_R = 0; out_R < out_rows; out_R++){
				const int in_R = out_R*stride + K_row - pad_rows;
				if (in_R >= 0 && in_R<in_rows){
					for (int out_C = 0; out_C < out_cols; out_C++){
						const int in_C = out_C*stride + K_col - pad_cols;
						if (in_C >= 0 && in_C<in_cols){
							int in_index = in_R*in_cols + in_C;
							int out_index = out_R*out_cols + out_C;
							gradient += inputdata_slice[in_index] * outputdata_slice[out_index];
						}
					}
				}
			}			
		}
		weight_diff[thread_id] = gradient;

	}
	



	
	/*�����̼߳���������ͨ��im2col��ʽ��������Ŀǰ�汾������group��ʽ
	*���巽ʽ������ʾ����Ҫ����Ĳ���
	* inputdata		     	��������
	* in_group				������ÿ����������ͨ����
	* in_rows				����ͼ��߶�
	* in_cols				����ͼ����
	* in_depth				����ͼ����ͨ����
	* weight                ���Ȩ��
	* weight_rows           ����˵ĸ߶�
	* weight_cols			����˵Ŀ��
	* stride                ����
	* pad_rows              pad��
	* pad_cols              pad��
	* biasexist             bias�Ƿ���ڵı�־
	* biasdata              bias����
	* out_group			 	���ͼ����ÿ����������ͨ����
	* out_rows				���ͼ��߶�
	* out_cols				���ͼ����
	* out_depth             ���ͼ����ͨ����
	* outputdata            ������ݣ�
	* out_Number            ��������ܳ���
	*/
	template <typename Dtype>
	__global__ void forward_convwithgroup(const Dtype *inputdata, const int in_group, const int in_rows, const int in_cols, const int in_depth,
		const Dtype *weight, const int weight_rows, const int weight_cols, const int stride, const int pad_rows, const int pad_cols, bool biasexist, const Dtype *biasdata,
		const int out_group, const int out_rows, const int out_cols, const int out_depth, Dtype *outputdata, const int out_Number){
		int thread_id = (blockIdx.x + blockIdx.y*gridDim.x) * blockDim.x + threadIdx.x;
		if (thread_id >= out_Number) return;
		const int OC = thread_id % out_cols;//width
		const int OR = (thread_id / out_cols) % out_rows;//height
		const int OD = (thread_id / out_cols / out_rows) % out_depth;//channel
		const int OB = thread_id / out_cols / out_rows / out_depth;//batch size

		const int input_row_start = OR * stride - pad_rows;
		const int input_col_start = OC * stride - pad_cols;
		const int input_row_end = input_row_start + weight_rows;
		const int input_col_end = input_col_start + weight_cols;

		/* in_groupΪ����ÿ����������ͨ����
		*out_groupΪ���ÿ����������ͨ����
		*���㷽ʽ�����ȸ���OD�������ǰͨ�����ڵڼ���Nth_group=OD/out_group;
		*����Nth_group��������inputdata��ƫ��in_offset=Nth_group*in_group*in_cols*in_rows;
		*Ȩ��weightƫ��kernel_offset=OD*in_group*kernel_cols*kernel_rows;
		*/
		int Nth_group = OD / out_group;
		const int inputoffset = OB*in_depth*in_rows*in_cols + Nth_group*in_group*in_cols*in_rows; // +OD*in_cols*in_rows;// +input_row_start*in_cols + input_col_start;
		const int kerneloffset = OD*in_group*weight_cols*weight_rows;

		Dtype sum = 0.f;
		//#pragma unroll
		for (int W_C = 0; W_C < in_group; W_C++){
			int InputOffset = inputoffset + W_C*in_rows*in_cols;
			int KernelOffset = kerneloffset + W_C*weight_cols*weight_rows;
			//#pragma unroll
			for (int W_H = 0; W_H < weight_rows; W_H++){

				int in_r = input_row_start + W_H;
				//#pragma unroll
				for (int W_W = 0; W_W < weight_cols; W_W++){
					int in_c = input_col_start + W_W;
					if (in_r >= 0 && in_r < in_rows && in_c >= 0 && in_c < in_cols) {
						int input_offset = InputOffset + in_r*in_cols + in_c;
						int weight_offset = KernelOffset + W_H*weight_cols + W_W;
						sum += (*(inputdata + input_offset))*(*(weight + weight_offset));
					}

				}
			}
		}
		if (biasexist) outputdata[thread_id] = sum + biasdata[OD];
		else outputdata[thread_id] = sum;
	}
	
	
	
//	template <>
//	void gpu_convwithgroup<float>(const float *inputdata, const int in_group, const int in_rows, const int in_cols, const int in_depth,
//		const float *weight, const int weight_rows, const int weight_cols, 
//		const int stride, const int pad_rows, const int pad_cols, 
//		bool biasexist, const float *biasdata,
//		const int out_group, const int out_rows, const int out_cols, const int out_depth, float *outputdata, const int out_Number){
//		int thread_id = (blockIdx.x + blockIdx.y*gridDim.x) * blockDim.x + threadIdx.x;
//		if (thread_id >= out_Number) return;
//		const int OC = thread_id % out_cols;//width
//		const int OR = (thread_id / out_cols) % out_rows;//height
//		const int OD = (thread_id / out_cols / out_rows) % out_depth;//channel
//		const int OB = thread_id / out_cols / out_rows / out_depth;//batch size
//
//		const int input_row_start = OR * stride - pad_rows;
//		const int input_col_start = OC * stride - pad_cols;
//		const int input_row_end = input_row_start + weight_rows;
//		const int input_col_end = input_col_start + weight_cols;
//
//		/* in_groupΪ����ÿ����������ͨ����
//		*out_groupΪ���ÿ����������ͨ����
//		*���㷽ʽ�����ȸ���OD�������ǰͨ�����ڵڼ���Nth_group=OD/out_group;
//		*����Nth_group��������inputdata��ƫ��in_offset=Nth_group*in_group*in_cols*in_rows;
//		*Ȩ��weightƫ��kernel_offset=OD*in_group*kernel_cols*kernel_rows;
//		*/
//		int Nth_group = OD / out_group;
//		const int inputoffset = OB*in_depth*in_rows*in_cols + Nth_group*in_group*in_cols*in_rows; // +OD*in_cols*in_rows;// +input_row_start*in_cols + input_col_start;
//		const int kerneloffset = OD*in_group*weight_cols*weight_rows;
//
//		float sum = 0.f;
////#pragma unroll
//		for (int W_C = 0; W_C < in_group; W_C++){
//			int InputOffset = inputoffset + W_C*in_rows*in_cols;
//			int KernelOffset = kerneloffset + W_C*weight_cols*weight_rows;
////#pragma unroll
//			for (int W_H = 0; W_H < weight_rows; W_H++){
//
//				int in_r = input_row_start + W_H;
////#pragma unroll
//				for (int W_W = 0; W_W < weight_cols; W_W++){
//					int in_c = input_col_start + W_W;
//					if (in_r >= 0 && in_r < in_rows && in_c >= 0 && in_c < in_cols) {
//						int input_offset = InputOffset + in_r*in_cols + in_c;
//						int weight_offset = KernelOffset + W_H*weight_cols + W_W;
//						sum += (*(inputdata + input_offset))*(*(weight + weight_offset));
//					}
//
//				}
//			}
//		}
//		if (biasexist) outputdata[thread_id] = sum + biasdata[OD];
//		else outputdata[thread_id] = sum;
//	}
//	template <>
//	void gpu_convwithgroup<double>(const double *inputdata, const int in_group, const int in_rows, const int in_cols, const int in_depth,
//		const double *weight, const int weight_rows, const int weight_cols, const int stride, const int pad_rows, const int pad_cols, bool biasexist, const double *biasdata,
//		const int out_group, const int out_rows, const int out_cols, const int out_depth, double *outputdata, const int out_Number){
//		int thread_id = (blockIdx.x + blockIdx.y*gridDim.x) * blockDim.x + threadIdx.x;
//		if (thread_id >= out_Number) return;
//		const int OC = thread_id % out_cols;//width
//		const int OR = (thread_id / out_cols) % out_rows;//height
//		const int OD = (thread_id / out_cols / out_rows) % out_depth;//channel
//		const int OB = thread_id / out_cols / out_rows / out_depth;//batch size
//
//		const int input_row_start = OR * stride - pad_rows;
//		const int input_col_start = OC * stride - pad_cols;
//		const int input_row_end = input_row_start + weight_rows;
//		const int input_col_end = input_col_start + weight_cols;
//
//		/* in_groupΪ����ÿ����������ͨ����
//		*out_groupΪ���ÿ����������ͨ����
//		*���㷽ʽ�����ȸ���OD�������ǰͨ�����ڵڼ���Nth_group=OD/out_group;
//		*����Nth_group��������inputdata��ƫ��in_offset=Nth_group*in_group*in_cols*in_rows;
//		*Ȩ��weightƫ��kernel_offset=OD*in_group*kernel_cols*kernel_rows;
//		*/
//		int Nth_group = OD / out_group;
//		const int inputoffset = OB*in_depth*in_rows*in_cols + Nth_group*in_group*in_cols*in_rows; // +OD*in_cols*in_rows;// +input_row_start*in_cols + input_col_start;
//		const int kerneloffset = OD*in_group*weight_cols*weight_rows;
//
//		double sum = 0.f;
////#pragma unroll
//		for (int W_C = 0; W_C < in_group; W_C++){
//			int InputOffset = inputoffset + W_C*in_rows*in_cols;
//			int KernelOffset = kerneloffset + W_C*weight_cols*weight_rows;
////#pragma unroll
//			for (int W_H = 0; W_H < weight_rows; W_H++){
//
//				int in_r = input_row_start + W_H;
////#pragma unroll
//				for (int W_W = 0; W_W < weight_cols; W_W++){
//					int in_c = input_col_start + W_W;
//					if (in_r >= 0 && in_r < in_rows && in_c >= 0 && in_c < in_cols) {
//						int input_offset = InputOffset + in_r*in_cols + in_c;
//						int weight_offset = KernelOffset + W_H*weight_cols + W_W;
//						sum += (*(inputdata + input_offset))*(*(weight + weight_offset));
//					}
//
//				}
//			}
//		}
//		if (biasexist) outputdata[thread_id] = sum + biasdata[OD];
//		else outputdata[thread_id] = sum;
//	};




	template <typename Dtype>
	void ZKConvolutionLayer<Dtype>::Forward_gpu(const vector<Blob<Dtype>*>& bottom,
		const vector<Blob<Dtype>*>& top) {
		const int group = this->group_;
		const Dtype* weight = this->blobs_[0]->gpu_data();
		const int weight_rows = this->blobs_[0]->height();
		const int weight_cols = this->blobs_[0]->width();

		bool biasexist = this->bias_term_;
		const Dtype *biasdata = NULL;
		if (biasexist) biasdata = this->blobs_[1]->gpu_data();


		int* stride_data = this->stride_.mutable_cpu_data();
		const int stride = stride_data[0];
		
		int *pad_data = this->pad_.mutable_cpu_data();
		const int pad_rows = pad_data[0];
		const int pad_cols = pad_data[1];

		for (int i = 0; i < bottom.size(); ++i) {
			const Dtype* bottom_data = bottom[i]->gpu_data();   //��������
			vector<int> in_shape_ = bottom[i]->shape();			
			const int in_channels_ = in_shape_[1];               //�������ݵ�ͨ����
			const int in_group = in_channels_ / group;        //����ÿ�������ͨ����
			const int in_height_ = in_shape_[2];				  //�������ݵĸ߶�	
			const int in_width_ = in_shape_[3];				  //�������ݵĿ��

			Dtype* top_data = top[i]->mutable_gpu_data();     // �������
			const int count = top[i]->count();				  // ������ݵ��ܳ���
			vector<int> outshape_ = top[i]->shape();         
			const int outchannels_ = outshape_[1];			  //������ݵ�ͨ����
			const int out_group = outchannels_ / group;		  //���ÿ�������ͨ����
			const int outheight_ = outshape_[2];			  //������ݵĸ߶�
			const int outwidth_ = outshape_[3];				  //������ݵĿ��
			
			forward_convwithgroup<Dtype> << <cuda_gridsize(count), CAFFE_CUDA_NUM_THREADS >> > (bottom_data, in_group, in_height_,
				in_width_, in_channels_, weight, weight_rows, weight_cols, stride, pad_rows, pad_cols, biasexist, biasdata,
				out_group, outheight_, outwidth_, outchannels_, top_data, count);
		}
		
	}

	template <typename Dtype>
	void ZKConvolutionLayer<Dtype>::Backward_gpu(const vector<Blob<Dtype>*>& top,
		const vector<bool>& propagate_down, const vector<Blob<Dtype>*>& bottom) {
		const int batchSize = this->num_;
		const Dtype* weight = this->blobs_[0]->gpu_data();
		Dtype* weight_diff = this->blobs_[0]->mutable_gpu_diff();
		const int weight_num = this->blobs_[0]->count();
		const int weight_rows = this->blobs_[0]->height();
		const int weight_cols = this->blobs_[0]->width();
		const int weight_depth = this->blobs_[0]->channels();

		int* stride_data = this->stride_.mutable_cpu_data();
		const int stride = stride_data[0];

		int *pad_data = this->pad_.mutable_cpu_data();
		const int pad_rows = pad_data[0];
		const int pad_cols = pad_data[1];

		/*const int top_rows, const int top_cols, const int top_depth,
			Dtype *bias_diff, const int bias_Num*/
		//Dtype* weight_diff = this->blobs_[0]->mutable_gpu_diff();
		for (int i = 0; i < top.size(); ++i) {
			const Dtype* top_diff = top[i]->gpu_diff();
			std::vector<int> outshape_ = top[i]->shape();
			const int bias_number = this->conv_out_channels_;
			const int outchannels_ = outshape_[1];			  //������ݵ�ͨ����
			const int out_group = outchannels_ / group_;		  //���ÿ�������ͨ����
			const int outheight_ = outshape_[2];			  //������ݵĸ߶�
			const int outwidth_ = outshape_[3];				  //������ݵĿ��
			// Bias gradient, if necessary.
			if (this->bias_term_ && this->param_propagate_down_[1]){
				Dtype* bias_diff = this->blobs_[1]->mutable_gpu_diff();				

				backward_bias_gpuwithgroup<Dtype> << <cuda_gridsize(bias_number), CAFFE_CUDA_NUM_THREADS >> >(batchSize, top_diff, outheight_,
					outwidth_, outchannels_, bias_diff, bias_number);
			}

			if (this->param_propagate_down_[0] || propagate_down[i]){
				const Dtype* bottom_data = bottom[i]->gpu_data();
				Dtype* bottom_diff = bottom[i]->mutable_gpu_diff();
				const int bottom_number = bottom[i]->count();
				vector<int> in_shape_ = bottom[i]->shape();
				const int in_channels_ = in_shape_[1];               //�������ݵ�ͨ����
				const int in_group = in_channels_ / group_;        //����ÿ�������ͨ����
				const int in_height_ = in_shape_[2];				  //�������ݵĸ߶�	
				const int in_width_ = in_shape_[3];				  //�������ݵĿ��
				
				if (this->param_propagate_down_[0]){

					/*backward_weight_gpuwithgroup(const int num, const Dtype *inputdata, const int in_group, const int in_rows, const int in_cols, const int in_depth,
						const Dtype *output_diff, const int out_group, const int out_rows, const int out_cols, const int out_depth,
						const int stride, const int pad_rows, const int pad_cols,
						const int weight_rows, const int weight_cols, const int weight_depth, Dtype *weight_diff, const int weight_Number)*/
					backward_weight_gpuwithgroup<Dtype> << < cuda_gridsize(weight_num), CAFFE_CUDA_NUM_THREADS >> > (batchSize,
						bottom_data, in_group, in_height_, in_width_, in_channels_,
						top_diff, out_group, outheight_, outwidth_, outchannels_,
						stride, pad_rows, pad_cols,
						weight_rows, weight_cols, weight_depth, weight_diff, weight_num
						);
				}				
				/*backward_Input_gpuwithgroup(const Dtype *top_diff, const int top_rows, const int top_cols, const int top_depth, const int top_group,
				const Dtype *weight, const int weight_rows, const int weight_cols, const int weight_depth,
				const int stride, const int pad_rows, const int pad_cols,
				const int bottom_rows, const int bottom_cols, const int bottom_depth, const int bottom_group, Dtype *bottom_diff, const int bottom_Number)*/
				if (propagate_down[i]) {
					backward_Input_gpuwithgroup<Dtype> << <cuda_gridsize(bottom_number), CAFFE_CUDA_NUM_THREADS >> >(top_diff, outheight_, outwidth_,
						outchannels_, out_group, weight, weight_rows, weight_cols, weight_depth, stride, pad_rows, pad_rows,
						in_height_, in_width_, in_channels_, in_group, bottom_diff, bottom_number);
				}
			}

		}

	}


	//template <typename Dtype>
	//void ZKConvolutionLayer<Dtype>::Backward_gpu(const vector<Blob<Dtype>*>& top,
	//	const vector<bool>& propagate_down, const vector<Blob<Dtype>*>& bottom) {
	//	const Dtype* weight = this->blobs_[0]->gpu_data();
	//	Dtype* weight_diff = this->blobs_[0]->mutable_gpu_diff();
	//	for (int i = 0; i < top.size(); ++i) {
	//		const Dtype* top_diff = top[i]->gpu_diff();
	//		// Bias gradient, if necessary.
	//		if (this->bias_term_ && this->param_propagate_down_[1]) {
	//			Dtype* bias_diff = this->blobs_[1]->mutable_gpu_diff();
	//			for (int n = 0; n < this->num_; ++n) {
	//				this->backward_gpu_bias(bias_diff, top_diff + n * this->top_dim_);
	//			}
	//		}
	//		if (this->param_propagate_down_[0] || propagate_down[i]) {
	//			const Dtype* bottom_data = bottom[i]->gpu_data();
	//			Dtype* bottom_diff = bottom[i]->mutable_gpu_diff();
	//			for (int n = 0; n < this->num_; ++n) {
	//				// gradient w.r.t. weight. Note that we will accumulate diffs.
	//				if (this->param_propagate_down_[0]) {
	//					this->weight_gpu_gemm(bottom_data + n * this->bottom_dim_,
	//						top_diff + n * this->top_dim_, weight_diff);
	//				}
	//				// gradient w.r.t. bottom data, if necessary.
	//				if (propagate_down[i]) {
	//					this->backward_gpu_gemm(top_diff + n * this->top_dim_, weight,
	//						bottom_diff + n * this->bottom_dim_);
	//				}
	//			}
	//		}
	//	}
	//}



	INSTANTIATE_LAYER_GPU_FUNCS(ZKConvolutionLayer);
}