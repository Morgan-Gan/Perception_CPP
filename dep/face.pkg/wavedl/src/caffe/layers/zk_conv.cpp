#include <vector>
#include <algorithm>

#include "caffe/layers/zk_conv.hpp"
#include "caffe/filler.hpp"
namespace caffe {

	template <typename Dtype>
	void ZKConvolutionLayer<Dtype>::compute_output_shape() {
		const int* kernel_shape_data = this->kernel_shape_.cpu_data();
		const int* stride_data = this->stride_.cpu_data();
		const int* pad_data = this->pad_.cpu_data();
		//const int* dilation_data = this->dilation_.cpu_data();
		this->output_shape_.clear();
		for (int i = 0; i < this->num_spatial_axes_; ++i) {
			// i + 1 to skip channel axis
			const int input_dim = this->input_shape(i + 1);
			const int kernel_extent = kernel_shape_data[i];// dilation_data[i] * (kernel_shape_data[i] - 1) + 1;
			const int output_dim = (input_dim + 2 * pad_data[i] - kernel_extent)
				/ stride_data[i] + 1;
			this->output_shape_.push_back(output_dim);
		}
	}

	template <typename Dtype>
	void ZKConvolutionLayer<Dtype>::LayerSetUp(const vector<Blob<Dtype>*>& bottom,
		const vector<Blob<Dtype>*>& top){
		ConvolutionParameter conv_param = this->layer_param_.convolution_param();
		channel_axis_ = bottom[0]->CanonicalAxisIndex(conv_param.axis());
		const int first_spatial_axis = channel_axis_ + 1;
		const int num_axes = bottom[0]->num_axes();
		num_spatial_axes_ = num_axes - first_spatial_axis;
		CHECK_GE(num_spatial_axes_, 0);
		vector<int> bottom_dim_blob_shape(1, num_spatial_axes_ + 1);
		vector<int> spatial_dim_blob_shape(1, std::max(num_spatial_axes_, 1));
		kernel_shape_.Reshape(spatial_dim_blob_shape);
		int* kernel_shape_data = kernel_shape_.mutable_cpu_data();
		if (conv_param.has_kernel_h() || conv_param.has_kernel_w()) {
			CHECK_EQ(num_spatial_axes_, 2)
				<< "kernel_h & kernel_w can only be used for 2D convolution.";
			CHECK_EQ(0, conv_param.kernel_size_size())
				<< "Either kernel_size or kernel_h/w should be specified; not both.";
			kernel_shape_data[0] = conv_param.kernel_h();
			kernel_shape_data[1] = conv_param.kernel_w();
		}
		else {
			const int num_kernel_dims = conv_param.kernel_size_size();
			CHECK(num_kernel_dims == 1 || num_kernel_dims == num_spatial_axes_)
				<< "kernel_size must be specified once, or once per spatial dimension "
				<< "(kernel_size specified " << num_kernel_dims << " times; "
				<< num_spatial_axes_ << " spatial dims).";
			for (int i = 0; i < num_spatial_axes_; ++i) {
				kernel_shape_data[i] =
					conv_param.kernel_size((num_kernel_dims == 1) ? 0 : i);
			}
		}
		for (int i = 0; i < num_spatial_axes_; ++i) {
			CHECK_GT(kernel_shape_data[i], 0) << "Filter dimensions must be nonzero.";
		}
		// Setup stride dimensions (stride_).
		stride_.Reshape(spatial_dim_blob_shape);
		int* stride_data = stride_.mutable_cpu_data();
		if (conv_param.has_stride_h() || conv_param.has_stride_w()) {
			CHECK_EQ(num_spatial_axes_, 2)
				<< "stride_h & stride_w can only be used for 2D convolution.";
			CHECK_EQ(0, conv_param.stride_size())
				<< "Either stride or stride_h/w should be specified; not both.";
			stride_data[0] = conv_param.stride_h();
			stride_data[1] = conv_param.stride_w();
		}
		else {
			const int num_stride_dims = conv_param.stride_size();
			CHECK(num_stride_dims == 0 || num_stride_dims == 1 ||
				num_stride_dims == num_spatial_axes_)
				<< "stride must be specified once, or once per spatial dimension "
				<< "(stride specified " << num_stride_dims << " times; "
				<< num_spatial_axes_ << " spatial dims).";
			const int kDefaultStride = 1;
			for (int i = 0; i < num_spatial_axes_; ++i) {
				stride_data[i] = (num_stride_dims == 0) ? kDefaultStride :
					conv_param.stride((num_stride_dims == 1) ? 0 : i);
				CHECK_GT(stride_data[i], 0) << "Stride dimensions must be nonzero.";
			}
		}
		// Setup pad dimensions (pad_).
		pad_.Reshape(spatial_dim_blob_shape);
		int* pad_data = pad_.mutable_cpu_data();
		if (conv_param.has_pad_h() || conv_param.has_pad_w()) {
			CHECK_EQ(num_spatial_axes_, 2)
				<< "pad_h & pad_w can only be used for 2D convolution.";
			CHECK_EQ(0, conv_param.pad_size())
				<< "Either pad or pad_h/w should be specified; not both.";
			pad_data[0] = conv_param.pad_h();
			pad_data[1] = conv_param.pad_w();
		}
		else {
			const int num_pad_dims = conv_param.pad_size();
			CHECK(num_pad_dims == 0 || num_pad_dims == 1 ||
				num_pad_dims == num_spatial_axes_)
				<< "pad must be specified once, or once per spatial dimension "
				<< "(pad specified " << num_pad_dims << " times; "
				<< num_spatial_axes_ << " spatial dims).";
			const int kDefaultPad = 0;
			for (int i = 0; i < num_spatial_axes_; ++i) {
				pad_data[i] = (num_pad_dims == 0) ? kDefaultPad :
					conv_param.pad((num_pad_dims == 1) ? 0 : i);
			}
		}
		// Configure output channels and groups.
		channels_ = bottom[0]->shape(channel_axis_);
		num_output_ = this->layer_param_.convolution_param().num_output();
		CHECK_GT(num_output_, 0);
		group_ = this->layer_param_.convolution_param().group();
		CHECK_EQ(channels_ % group_, 0);
		CHECK_EQ(num_output_ % group_, 0)
			<< "Number of output should be multiples of group.";
		if (reverse_dimensions()) {
			conv_out_channels_ = channels_;
			conv_in_channels_ = num_output_;
		}
		else {
			conv_out_channels_ = num_output_;
			conv_in_channels_ = channels_;
		}
		// Handle the parameters: weights and biases.
		// - blobs_[0] holds the filter weights
		// - blobs_[1] holds the biases (optional)
		vector<int> weight_shape(2);
		weight_shape[0] = conv_out_channels_;
		weight_shape[1] = conv_in_channels_ / group_;
		for (int i = 0; i < num_spatial_axes_; ++i) {
			weight_shape.push_back(kernel_shape_data[i]);
		}
		bias_term_ = this->layer_param_.convolution_param().bias_term();
		vector<int> bias_shape(bias_term_, num_output_);
		if (this->blobs_.size() > 0) {
			CHECK_EQ(1 + bias_term_, this->blobs_.size())
				<< "Incorrect number of weight blobs.";
			if (weight_shape != this->blobs_[0]->shape()) {
				Blob<Dtype> weight_shaped_blob(weight_shape);
				LOG(FATAL) << "Incorrect weight shape: expected shape "
					<< weight_shaped_blob.shape_string() << "; instead, shape was "
					<< this->blobs_[0]->shape_string();
			}
			if (bias_term_ && bias_shape != this->blobs_[1]->shape()) {
				Blob<Dtype> bias_shaped_blob(bias_shape);
				LOG(FATAL) << "Incorrect bias shape: expected shape "
					<< bias_shaped_blob.shape_string() << "; instead, shape was "
					<< this->blobs_[1]->shape_string();
			}
			LOG(INFO) << "Skipping parameter initialization";
		}
		else {
			if (bias_term_) this->blobs_.resize(2);
			else  this->blobs_.resize(1);

			// Initialize and fill the weights:
			// output channels x input channels per-group x kernel height x kernel width
			this->blobs_[0].reset(new Blob<Dtype>(weight_shape));
			shared_ptr<Filler<Dtype> > weight_filler(GetFiller<Dtype>(
				this->layer_param_.convolution_param().weight_filler()));
			weight_filler->Fill(this->blobs_[0].get());
			// If necessary, initialize and fill the biases.
			if (bias_term_) {
				this->blobs_[1].reset(new Blob<Dtype>(bias_shape));
				shared_ptr<Filler<Dtype> > bias_filler(GetFiller<Dtype>(
					this->layer_param_.convolution_param().bias_filler()));
				bias_filler->Fill(this->blobs_[1].get());
			}
		}
		kernel_dim_ = this->blobs_[0]->count(1);
		weight_offset_ = conv_out_channels_ * kernel_dim_ / group_;
		// Propagate gradients to the parameters (as directed by backward pass).
		this->param_propagate_down_.resize(this->blobs_.size(), true);		
	}
	
	template <typename Dtype>
	void ZKConvolutionLayer<Dtype>::Reshape(const vector<Blob<Dtype>*>& bottom,
		const vector<Blob<Dtype>*>& top){
		const int first_spatial_axis = channel_axis_ + 1;
		CHECK_EQ(bottom[0]->num_axes(), first_spatial_axis + num_spatial_axes_)
			<< "bottom num_axes may not change.";
		num_ = bottom[0]->count(0, channel_axis_);
		CHECK_EQ(bottom[0]->shape(channel_axis_), channels_)
			<< "Input size incompatible with convolution kernel.";
		// TODO: generalize to handle inputs of different shapes.
		for (int bottom_id = 1; bottom_id < bottom.size(); ++bottom_id) {
			CHECK(bottom[0]->shape() == bottom[bottom_id]->shape())
				<< "All inputs must have the same shape.";
		}
		bottom_shape_ = &bottom[0]->shape();
		compute_output_shape();
		vector<int> top_shape(bottom[0]->shape().begin(),
			bottom[0]->shape().begin() + channel_axis_);
		top_shape.push_back(num_output_);
		for (int i = 0; i < num_spatial_axes_; ++i) {
			top_shape.push_back(output_shape_[i]);
		}
		for (int top_id = 0; top_id < top.size(); ++top_id) {
			top[top_id]->Reshape(top_shape);
		}
	}

	template <typename Dtype>
	void ZKConvolutionLayer<Dtype>::Forward_cpu(const vector<Blob<Dtype>*>& bottom,
		const vector<Blob<Dtype>*>& top) {
		
		return;

		/*const Dtype* weight = this->blobs_[0]->cpu_data();
		for (int i = 0; i < bottom.size(); ++i) {
			const Dtype* bottom_data = bottom[i]->cpu_data();
			Dtype* top_data = top[i]->mutable_cpu_data();
			for (int n = 0; n < this->num_; ++n) {
				this->forward_cpu_gemm(bottom_data + n * this->bottom_dim_, weight,
					top_data + n * this->top_dim_);
				if (this->bias_term_) {
					const Dtype* bias = this->blobs_[1]->cpu_data();
					this->forward_cpu_bias(top_data + n * this->top_dim_, bias);
				}
			}
		}*/
	}

	template <typename Dtype>
	void ZKConvolutionLayer<Dtype>::Backward_cpu(const vector<Blob<Dtype>*>& top,
		const vector<bool>& propagate_down, const vector<Blob<Dtype>*>& bottom) {
		
		return;

		//const Dtype* weight = this->blobs_[0]->cpu_data();
		//Dtype* weight_diff = this->blobs_[0]->mutable_cpu_diff();
		//for (int i = 0; i < top.size(); ++i) {
		//	const Dtype* top_diff = top[i]->cpu_diff();
		//	const Dtype* bottom_data = bottom[i]->cpu_data();
		//	Dtype* bottom_diff = bottom[i]->mutable_cpu_diff();
		//	// Bias gradient, if necessary.
		//	if (this->bias_term_ && this->param_propagate_down_[1]) {
		//		Dtype* bias_diff = this->blobs_[1]->mutable_cpu_diff();
		//		for (int n = 0; n < this->num_; ++n) {
		//			this->backward_cpu_bias(bias_diff, top_diff + n * this->top_dim_);
		//		}
		//	}
		//	if (this->param_propagate_down_[0] || propagate_down[i]) {
		//		for (int n = 0; n < this->num_; ++n) {
		//			// gradient w.r.t. weight. Note that we will accumulate diffs.
		//			if (this->param_propagate_down_[0]) {
		//				this->weight_cpu_gemm(bottom_data + n * this->bottom_dim_,
		//					top_diff + n * this->top_dim_, weight_diff);
		//			}
		//			// gradient w.r.t. bottom data, if necessary.
		//			if (propagate_down[i]) {
		//				this->backward_cpu_gemm(top_diff + n * this->top_dim_, weight,
		//					bottom_diff + n * this->bottom_dim_);
		//			}
		//		}
		//	}
		//}
	}

#ifdef CPU_ONLY
	STUB_GPU(ZKConvolutionLayer);
#endif
	INSTANTIATE_CLASS(ZKConvolutionLayer);
	REGISTER_LAYER_CLASS(ZKConvolution);
}