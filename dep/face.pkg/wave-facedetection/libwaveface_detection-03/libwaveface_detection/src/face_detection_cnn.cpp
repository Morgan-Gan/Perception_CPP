
#include "face_detection_cnn.h"

using namespace caffe;  // NOLINT(build/namespaces)
using std::shared_ptr;

cv::Rect BBox::GetRect() const {
   return cv::Rect(x1, y1, x2 - x1, y2 - y1);
}

BBox BBox::GetSquare() const {
   BBox bbox;
   float bbox_width = x2 - x1;
   float bbox_height = y2 - y1;
   float side = std::max(bbox_width, bbox_height);
   bbox.x1 = static_cast<int>(x1 + (bbox_width - side) * 0.5f);
   bbox.y1 = static_cast<int>(y1 + (bbox_height - side) * 0.5f);
   bbox.x2 = static_cast<int>(bbox.x1 + side);
   bbox.y2 = static_cast<int>(bbox.y1 + side);
   return bbox;
 }

void Face::Bboxes2Squares(std::vector<Face>& faces) {
  for (size_t i = 0; i < faces.size(); ++i) {
    faces[i].bbox = faces[i].bbox.GetSquare();
  }
}

FaceDetector::FaceDetector():num_channels_(3),
  threshold_(0.9),
  gpu_id_(-1),
  need_memory_(450){
  min_face_size_ = 14;
}

FaceDetector::~FaceDetector() {
}


int FaceDetector::Initial(const string& path,
                          const int& min_face_size,
                          const float &threshold,
                          const int &gpu_id) {
  SetThreshold(threshold);
  gpu_id_ = gpu_id;
  
  if (gpu_id_ < 0) {
    Caffe::set_mode(Caffe::CPU);
  } else {
    Caffe::set_mode(Caffe::GPU);
    Caffe::SetDevice(gpu_id_);
  }
  std::string model_file = path + "/face_detection_centos64_v2.0.0.wave";

  /* Load the network. */
  caffe::NetParameter params;
  if (!ReadProtoFromBinaryFile(model_file, &params)) {
    return -1;
  }

  params.mutable_state()->set_phase(caffe::TEST);
  net_.reset(new Net<float>(params));

  Blob<float>* input_layer = net_->input_blobs()[0];
  num_channels_ = input_layer->channels();
  input_geometry_ = cv::Size(input_layer->width(), input_layer->height());
  /* Load the binaryproto mean file. */
  SetMinFaceSize(min_face_size);

  std::string mean_value = "104,117,123";
  SetMean(mean_value);

  return 0;
}

std::vector<Face> FaceDetector::Detect(const cv::Mat& img) { 
  if (gpu_id_ >= 0) {
    caffe::Caffe::set_mode(caffe::Caffe::GPU);
    caffe::Caffe::SetDevice(gpu_id_);
  }

  int input_width_resize = int(float(img.cols) * (14.0f / float(min_face_size_)));
  int input_height_resize = int((float(img.rows) / float(img.cols)) * input_width_resize);

  if ((input_width_resize != input_geometry_.width) ||
      (input_height_resize != input_geometry_.height)) {

    input_geometry_.width = input_width_resize;
    input_geometry_.height = input_height_resize;

//    printf("(width, height) ->  %d  %d", input_width_resize, input_height_resize);

    Blob<float>* input_layer = net_->input_blobs()[0];
    input_layer->Reshape(1, num_channels_,
                      input_geometry_.height, input_geometry_.width);
    /* Forward dimension change to all layers. */
    net_->Reshape();

    std::string mean_value = "104,117,123";
    SetMean(mean_value);
  }


  std::vector<cv::Mat> input_channels;
  WrapInputLayer(&input_channels);
  Preprocess(img, &input_channels);

  net_->Forward();

  /* Copy the output layer to a std::vector */
  Blob<float>* result_blob = net_->output_blobs()[0];
  const float* result = result_blob->cpu_data();
  const int num_det = result_blob->height();
  vector<vector<float> > detections;
  for (int k = 0; k < num_det; ++k) {
    if (result[0] == -1) {
      // Skip invalid detection.
      result += 7;
      continue;
    }
    vector<float> detection(result, result + 7);
    detections.push_back(detection);
    result += 7;
  }
  std::vector<Face> faces;
  /* Print the detection results. */
  for (int i = 0; i < detections.size(); ++i) {
     const vector<float>& d = detections[i];
     // Detection format: [image_id, label, score, xmin, ymin, xmax, ymax].
       const float score = d[2];
     if (score >= threshold_) {
        Face face;
        face.bbox.x1 = d[3] * img.cols;
        face.bbox.y1 = d[4] * img.rows;
        face.bbox.x2 = d[5] * img.cols;
        face.bbox.y2 = d[6] * img.rows;
        if (face.bbox.x2-face.bbox.x1 < min_face_size_) {
           continue;
        }
        face.score = score;
        faces.push_back(face);
     }
  }

  return faces;
}

/* Load the mean file in binaryproto format. */
void FaceDetector::SetMean(const string& mean_value) {
  cv::Scalar channel_mean;
  if (!mean_value.empty()) {
    stringstream ss(mean_value);
    vector<float> values;
    string item;
    while (getline(ss, item, ',')) {
      float value = std::atof(item.c_str());
      values.push_back(value);
    }

    std::vector<cv::Mat> channels;
    for (int i = 0; i < num_channels_; ++i) {
      /* Extract an individual channel. */
      cv::Mat channel(input_geometry_.height, input_geometry_.width, CV_32FC1,
          cv::Scalar(values[i]));
      channels.push_back(channel);
    }
    cv::merge(channels, mean_);
  }
}

/* Wrap the input layer of the network in separate cv::Mat objects
 * (one per channel). This way we save one memcpy operation and we
 * don't need to rely on cudaMemcpy2D. The last preprocessing
 * operation will write the separate channels directly to the input
 * layer. */
void FaceDetector::WrapInputLayer(std::vector<cv::Mat>* input_channels) {
  Blob<float>* input_layer = net_->input_blobs()[0];

  int width = input_layer->width();
  int height = input_layer->height();
  float* input_data = input_layer->mutable_cpu_data();
  for (int i = 0; i < input_layer->channels(); ++i) {
    cv::Mat channel(height, width, CV_32FC1, input_data);
    input_channels->push_back(channel);
    input_data += width * height;
  }
}

void FaceDetector::Preprocess(const cv::Mat& img,
                            std::vector<cv::Mat>* input_channels) {
  /* Convert the input image to the input image format of the network. */
  cv::Mat sample;
  if (img.channels() == 3 && num_channels_ == 1)
    cv::cvtColor(img, sample, cv::COLOR_BGR2GRAY);
  else if (img.channels() == 4 && num_channels_ == 1)
    cv::cvtColor(img, sample, cv::COLOR_BGRA2GRAY);
  else if (img.channels() == 4 && num_channels_ == 3)
    cv::cvtColor(img, sample, cv::COLOR_BGRA2BGR);
  else if (img.channels() == 1 && num_channels_ == 3)
    cv::cvtColor(img, sample, cv::COLOR_GRAY2BGR);
  else
    sample = img;
  cv::Mat sample_resized;
  if (sample.size() != input_geometry_) {
    cv::resize(sample, sample_resized, input_geometry_);
  } else {
    sample_resized = sample;
  }
  cv::Mat sample_float;
  if (num_channels_ == 3)
    sample_resized.convertTo(sample_float, CV_32FC3);
  else
    sample_resized.convertTo(sample_float, CV_32FC1);

  cv::Mat sample_normalized;
  cv::subtract(sample_float, mean_, sample_normalized);

  /* This operation will write the separate BGR planes directly to the
   * input layer of the network because it is wrapped by the cv::Mat
   * objects in input_channels. */
  cv::split(sample_normalized, *input_channels);
}


int FaceDetector::Release() {
  return 0;
}
