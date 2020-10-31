#ifndef CAFFE_ZK_CONVOLUTION_LAYER_HPP_
#define CAFFE_ZK_CONVOLUTION_LAYER_HPP_



#include <vector>

#include "caffe/blob.hpp"
#include "caffe/layer.hpp"
#include "caffe/proto/caffe.pb.h"

//#include "caffe/layers/base_conv_layer.hpp"
#include "caffe/util/device_alternate.hpp"
#include "caffe/util/mkl_alternate.hpp"


namespace caffe {	
	/*template <typename Dtype>
	__global__ void gpu_convwithgroup(const Dtype *inputdata, const int in_group, const int in_rows, const int in_cols, const int in_depth,
		const Dtype *weight, const int weight_rows, const int weight_cols, const int stride, const int pad_rows, const int pad_cols, bool biasexist, const Dtype *biasdata,
		const int out_group, const int out_rows, const int out_cols, const int out_depth, Dtype *outputdata, const int out_Number);*/
	/**
	* @brief Convolves the input image with a bank of learned filters,
	*        and (optionally) adds biases.
	*
	*   Caffe convolves by reduction to matrix multiplication. This achieves
	*   high-throughput and generality of input and filter dimensions but comes at
	*   the cost of memory for matrices. This makes use of efficiency in BLAS.
	*
	*   The input is "im2col" transformed to a channel K' x H x W data matrix
	*   for multiplication with the N x K' x H x W filter matrix to yield a
	*   N' x H x W output matrix that is then "col2im" restored. K' is the
	*   input channel * kernel height * kernel width dimension of the unrolled
	*   inputs so that the im2col matrix has a column for each input region to
	*   be filtered. col2im restores the output spatial structure by rolling up
	*   the output channel N' columns of the output matrix.
	*/
	template <typename Dtype>
	class ZKConvolutionLayer : public Layer<Dtype> {
	public:
		/**
		* @param param provides ConvolutionParameter convolution_param,
		*    with ConvolutionLayer options:
		*  - num_output. The number of filters.
		*  - kernel_size / kernel_h / kernel_w. The filter dimensions, given by
		*  kernel_size for square filters or kernel_h and kernel_w for rectangular
		*  filters.
		*  - stride / stride_h / stride_w (\b optional, default 1). The filter
		*  stride, given by stride_size for equal dimensions or stride_h and stride_w
		*  for different strides. By default the convolution is dense with stride 1.
		*  - pad / pad_h / pad_w (\b optional, default 0). The zero-padding for
		*  convolution, given by pad for equal dimensions or pad_h and pad_w for
		*  different padding. Input padding is computed implicitly instead of
		*  actually padding.
		*  - dilation (\b optional, default 1). The filter
		*  dilation, given by dilation_size for equal dimensions for different
		*  dilation. By default the convolution has dilation 1.
		*  - group (\b optional, default 1). The number of filter groups. Group
		*  convolution is a method for reducing parameterization by selectively
		*  connecting input and output channels. The input and output channel dimensions must be divisible
		*  by the number of groups. For group @f$ \geq 1 @f$, the
		*  convolutional filters' input and output channels are separated s.t. each
		*  group takes 1 / group of the input channels and makes 1 / group of the
		*  output channels. Concretely 4 input channels, 8 output channels, and
		*  2 groups separate input channels 1-2 and output channels 1-4 into the
		*  first group and input channels 3-4 and output channels 5-8 into the second
		*  group.
		*  - bias_term (\b optional, default true). Whether to have a bias.
		*  - engine: convolution has CAFFE (matrix multiplication) and CUDNN (library
		*    kernels + stream parallelism) engines.
		*/
		explicit ZKConvolutionLayer(const LayerParameter& param)
			: Layer<Dtype>(param) {}
		virtual void LayerSetUp(const vector<Blob<Dtype>*>& bottom,
			const vector<Blob<Dtype>*>& top);
		virtual void Reshape(const vector<Blob<Dtype>*>& bottom,
			const vector<Blob<Dtype>*>& top);

		virtual inline int ExactNumBottomBlobs() const { return 1; }
		virtual inline int ExactNumTopBlobs() const { return 1; }
		virtual inline const char* type() const { return "ZKConvolution"; }

	protected:
		virtual void Forward_cpu(const vector<Blob<Dtype>*>& bottom,
			const vector<Blob<Dtype>*>& top);
		virtual void Forward_gpu(const vector<Blob<Dtype>*>& bottom,
			const vector<Blob<Dtype>*>& top);
		virtual void Backward_cpu(const vector<Blob<Dtype>*>& top,
			const vector<bool>& propagate_down, const vector<Blob<Dtype>*>& bottom);
		virtual void Backward_gpu(const vector<Blob<Dtype>*>& top,
			const vector<bool>& propagate_down, const vector<Blob<Dtype>*>& bottom);
		virtual inline bool reverse_dimensions() { return false; }
		virtual void compute_output_shape();

		inline int input_shape(int i) {
			return (*bottom_shape_)[channel_axis_ + i];
		}

		Blob<int> kernel_shape_;
		/// @brief The spatial dimensions of the stride.
		Blob<int> stride_;
		/// @brief The spatial dimensions of the padding.
		Blob<int> pad_;
		/// @brief The spatial dimensions of the dilation.
		//Blob<int> dilation_;
		/// @brief The spatial dimensions of the convolution input.
		Blob<int> conv_input_shape_;
		/// @brief The spatial dimensions of the col_buffer.
		///vector<int> col_buffer_shape_;
		/// @brief The spatial dimensions of the output.
		vector<int> output_shape_;
		const vector<int>* bottom_shape_;

		
		int num_spatial_axes_;
		int bottom_dim_;
		int top_dim_;

		int channel_axis_;
		int num_;
		int channels_;
		int group_;
		int out_spatial_dim_;
		int weight_offset_;
		int num_output_;
		bool bias_term_;
		//bool is_1x1_;
		//bool force_nd_im2col_;

		//template <typename Dtype>
		//int num_kernels_im2col_;
		//int num_kernels_col2im_;
		int conv_out_channels_;
		int conv_in_channels_;
		int conv_out_spatial_dim_;
		int kernel_dim_;
		int col_offset_;
		int output_offset_;
	};

}
#endif