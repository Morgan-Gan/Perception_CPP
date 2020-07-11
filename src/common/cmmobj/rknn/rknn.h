#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <sys/time.h>

#include "rknn/rknn_api.h"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace std;
using namespace cv;

#define NUM_RESULTS         1917
#define NUM_CLASSES         91

#define Y_SCALE  10.0f
#define X_SCALE  10.0f
#define H_SCALE  5.0f
#define W_SCALE  5.0f

Scalar colorArray[10] = {
        Scalar(139,   0,   0, 255),
        Scalar(139,   0, 139, 255),
        Scalar(  0,   0, 139, 255),
        Scalar(  0, 100,   0, 255),
        Scalar(139, 139,   0, 255),
        Scalar(209, 206,   0, 255),
        Scalar(  0, 127, 255, 255),
        Scalar(139,  61,  72, 255),
        Scalar(  0, 255,   0, 255),
        Scalar(255,   0,   0, 255),
};

float MIN_SCORE = 0.4f;

float NMS_THRESHOLD = 0.45f;

int loadLabelName(string locationFilename, string* labels) 
{
    ifstream fin(locationFilename);
    string line;
    int lineNum = 0;
    while(getline(fin, line))
    {
        labels[lineNum] = line;
        lineNum++;
    }
    return 0;
}

int loadCoderOptions(string locationFilename, float (*boxPriors)[NUM_RESULTS])
{
    const char *d = ", ";
    ifstream fin(locationFilename);
    string line;
    int lineNum = 0;
    while(getline(fin, line))
    {
        char *line_str = const_cast<char *>(line.c_str());
        char *p;
        p = strtok(line_str, d);
        int priorIndex = 0;
        while (p) {
            float number = static_cast<float>(atof(p));
            boxPriors[lineNum][priorIndex++] = number;
            p=strtok(nullptr, d);
        }
        if (priorIndex != NUM_RESULTS) {
            return -1;
        }
        lineNum++;
    }
    return 0;

}

float CalculateOverlap(float xmin0, float ymin0, float xmax0, float ymax0, float xmin1, float ymin1, float xmax1, float ymax1) 
{
    float w = max(0.f, min(xmax0, xmax1) - max(xmin0, xmin1));
    float h = max(0.f, min(ymax0, ymax1) - max(ymin0, ymin1));
    float i = w * h;
    float u = (xmax0 - xmin0) * (ymax0 - ymin0) + (xmax1 - xmin1) * (ymax1 - ymin1) - i;
    return u <= 0.f ? 0.f : (i / u);
}

float expit(float x)
{
    return (float) (1.0 / (1.0 + exp(-x)));
}

void decodeCenterSizeBoxes(float* predictions, float (*boxPriors)[NUM_RESULTS]) 
{
    for (int i = 0; i < NUM_RESULTS; ++i) {
        float ycenter = predictions[i*4+0] / Y_SCALE * boxPriors[2][i] + boxPriors[0][i];
        float xcenter = predictions[i*4+1] / X_SCALE * boxPriors[3][i] + boxPriors[1][i];
        float h = (float) exp(predictions[i*4 + 2] / H_SCALE) * boxPriors[2][i];
        float w = (float) exp(predictions[i*4 + 3] / W_SCALE) * boxPriors[3][i];

        float ymin = ycenter - h / 2.0f;
        float xmin = xcenter - w / 2.0f;
        float ymax = ycenter + h / 2.0f;
        float xmax = xcenter + w / 2.0f;

        predictions[i*4 + 0] = ymin;
        predictions[i*4 + 1] = xmin;
        predictions[i*4 + 2] = ymax;
        predictions[i*4 + 3] = xmax;
    }
}

int scaleToInputSize(float * outputClasses, int (*output)[NUM_RESULTS], int numClasses)
{
    int validCount = 0;
    // Scale them back to the input size.
    for (int i = 0; i < NUM_RESULTS; ++i) {
        float topClassScore = static_cast<float>(-1000.0);
        int topClassScoreIndex = -1;

        // Skip the first catch-all class.
        for (int j = 1; j < numClasses; ++j) {
            float score = expit(outputClasses[i*numClasses+j]);
            if (score > topClassScore) {
                topClassScoreIndex = j;
                topClassScore = score;
            }
        }

        if (topClassScore >= MIN_SCORE) {
            output[0][validCount] = i;
            output[1][validCount] = topClassScoreIndex;
            ++validCount;
        }
    }

    return validCount;
}

int nms(int validCount, float* outputLocations, int (*output)[NUM_RESULTS])
{
    for (int i=0; i < validCount; ++i) {
        if (output[0][i] == -1) {
            continue;
        }
        int n = output[0][i];
        for (int j=i + 1; j<validCount; ++j) {
            int m = output[0][j];
            if (m == -1) {
                continue;
            }
            float xmin0 = outputLocations[n*4 + 1];
            float ymin0 = outputLocations[n*4 + 0];
            float xmax0 = outputLocations[n*4 + 3];
            float ymax0 = outputLocations[n*4 + 2];

            float xmin1 = outputLocations[m*4 + 1];
            float ymin1 = outputLocations[m*4 + 0];
            float xmax1 = outputLocations[m*4 + 3];
            float ymax1 = outputLocations[m*4 + 2];

            float iou = CalculateOverlap(xmin0, ymin0, xmax0, ymax0, xmin1, ymin1, xmax1, ymax1);

            if (iou >= NMS_THRESHOLD) {
                output[0][j] = -1;
            }
        }
    }

    return 0;
}

template <class T>
void get_top_n(T* prediction, int prediction_size, size_t num_results,
               float threshold, std::vector<std::pair<float, int>>* top_results,
               bool input_floating) 
{
  // Will contain top N results in ascending order.
  std::priority_queue<pair<float, int>,vector<std::pair<float, int>>,greater<pair<float, int>>> top_result_pq;

  const long count = prediction_size;  // NOLINT(runtime/int)
  for (int i = 0; i < count; ++i) 
  {
    float value;
    if (input_floating)
      value = prediction[i];
    else
      value = prediction[i] / 255.0;

    // Only add it if it beats the threshold and has a chance at being in
    // the top N.
    if (value < threshold) 
    {
      continue;
    }

    top_result_pq.push(std::pair<float, int>(value, i));

    // If at capacity, kick the smallest value out.
    if (top_result_pq.size() > num_results) 
    {
      top_result_pq.pop();
    }
  }

  // Copy to output vector and reverse into descending order.
  while (!top_result_pq.empty()) {
    top_results->push_back(top_result_pq.top());
    top_result_pq.pop();
  }
  std::reverse(top_results->begin(), top_results->end());
}

int ReadLabelsFile(const string& file_name,std::vector<string>* result,size_t* found_label_count) 
{
  std::ifstream file(file_name);
  if (!file) 
  {
    std::cerr << "Labels file " << file_name << " not found\n";
    return -1;
  }

  result->clear();
  string line;
  while (std::getline(file, line)) 
  {
    result->push_back(line);
  }

  *found_label_count = result->size();
  const int padding = 16;
  while (result->size() % padding) 
  {
    result->emplace_back();
  }
  return 0;
}

int rknn_mobilenet()
{
    const char *img_path = "/tmp/dog.jpg";
    const char *model_path = "/tmp/mobilenet_v1-tf.rknn";
    const char *lable_path = "/tmp/labels.txt";
    const int output_elems = 1001;

    const int img_width = 224;
    const int img_height = 224;
    const int img_channels = 3;

    const int input_index = 0;      // node name "input"
    const int output_index = 0;     // node name "MobilenetV1/Predictions/Reshape_1"

    // Load image
    cv::Mat img = cv::imread(img_path, 1);
    if(!img.data) {
        printf("cv::imread %s fail!\n", img_path);
        return -1;
    }
    if(img.cols != img_width || img.rows != img_height)
        cv::resize(img, img, cv::Size(img_width, img_height), (0, 0), (0, 0), cv::INTER_LINEAR);

    //BGR->RGB
    cv::cvtColor(img, img, cv::COLOR_BGR2RGB);

    // Load model
    FILE *fp = fopen(model_path, "rb");
    if(fp == NULL) {
        printf("fopen %s fail!\n", model_path);
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    int model_len = ftell(fp);
    void *model = malloc(model_len);
    fseek(fp, 0, SEEK_SET);
    if(model_len != fread(model, 1, model_len, fp)) {
        printf("fread %s fail!\n", model_path);
        free(model);
        return -1;
    }

    // Start Inference
    rknn_input inputs[1];
    rknn_output outputs[1];
    rknn_tensor_attr output0_attr;

    int ret = 0;
    rknn_context ctx = 0;

    ret = rknn_init(&ctx, model, model_len, RKNN_FLAG_PRIOR_MEDIUM | RKNN_FLAG_COLLECT_PERF_MASK);
    if(ret < 0) {
        printf("rknn_init fail! ret=%d\n", ret);
        goto Error;
    }

    output0_attr.index = 0;
    ret = rknn_query(ctx, RKNN_QUERY_OUTPUT_ATTR, &output0_attr, sizeof(output0_attr));
    if(ret < 0) {
        printf("rknn_query fail! ret=%d\n", ret);
        goto Error;
    }

    inputs[0].index = input_index;
    inputs[0].buf = img.data;
    inputs[0].size = img_width * img_height * img_channels;
    inputs[0].pass_through = false;
    inputs[0].type = RKNN_TENSOR_UINT8;
    inputs[0].fmt = RKNN_TENSOR_NHWC;
    ret = rknn_inputs_set(ctx, 1, inputs);
    if(ret < 0) {
        printf("rknn_input_set fail! ret=%d\n", ret);
        goto Error;
    }

    ret = rknn_run(ctx, nullptr);
    if(ret < 0) {
        printf("rknn_run fail! ret=%d\n", ret);
        goto Error;
    }

    outputs[0].want_float = true;
    outputs[0].is_prealloc = false;
    ret = rknn_outputs_get(ctx, 1, outputs, nullptr);
    if(ret < 0) {
        printf("rknn_outputs_get fail! ret=%d\n", ret);
        goto Error;
    }

    // Process output
    if(outputs[0].size == output0_attr.n_elems * sizeof(float))
    {
        const size_t num_results = 5;
        const float threshold = 0.001f;

        std::vector<std::pair<float, int>> top_results;
        get_top_n<float>((float*)outputs[0].buf, output_elems,
                           num_results, threshold, &top_results, true);

        std::vector<string> labels;
        size_t label_count;
        if (!ReadLabelsFile(lable_path, &labels, &label_count)) {
            for (const auto& result : top_results) {
                const float confidence = result.first;
                const int index = result.second;
                std::cout << confidence << ": " << index << " " << labels[index] << "\n";
            }
        }
    }
    else
    {
        printf("rknn_outputs_get fail! get output_size = [%d], but expect %u!\n",
            outputs[0].size, (uint32_t)(output0_attr.n_elems * sizeof(float)));
    }

    rknn_outputs_release(ctx, 1, outputs);

    // performance query
    {
        rknn_perf_run perf_run;
        ret = rknn_query(ctx, RKNN_QUERY_PERF_RUN, &perf_run, sizeof(perf_run));
        if(ret < 0) {
            printf("rknn_query fail! ret=%d\n", ret);
            goto Error;
        }
        printf("perf_run.run_duration = %ld us\n", perf_run.run_duration);

        rknn_perf_detail perf_detail;
        ret = rknn_query(ctx, RKNN_QUERY_PERF_DETAIL, &perf_detail, sizeof(perf_detail));
        if(ret < 0) {
            printf("rknn_query fail! ret=%d\n", ret);
            goto Error;
        }
        if(perf_detail.data_len > 0 && perf_detail.perf_data)
            printf("perf_run.perf_data = %s\n", perf_detail.perf_data);
        else
            printf("please enable RKNN_FLAG_COLLECT_PERF_MASK flag in rknn_init!!!\n");
    }

    return 0;
Error:
    if(ctx > 0)         rknn_destroy(ctx);
    if(model)           free(model);
    if(fp)              fclose(fp);
    return 0;
}

int rknn_ssd()
{
    const char *img_path = "/tmp/road.bmp";
    const char *model_path = "/tmp/mobilenet_ssd.rknn";
    const char *label_path = "/tmp/coco_labels_list.txt";
    const char *box_priors_path = "/tmp/box_priors.txt";

    const int img_width = 300;
    const int img_height = 300;
    const int img_channels = 3;
    const int input_index = 0;      // node name "Preprocessor/sub"

    const int output_elems1 = NUM_RESULTS * 4;
    const uint32_t output_size1 = output_elems1 * sizeof(float);
    const int output_index1 = 0;    // node name "concat"

    const int output_elems2 = NUM_RESULTS * NUM_CLASSES;
    const uint32_t output_size2 = output_elems2 * sizeof(float);
    const int output_index2 = 1;    // node name "concat_1"

    // Load image
    cv::Mat img = cv::imread(img_path, 1);
    if(!img.data) {
        printf("cv::imread %s fail!\n", img_path);
        return -1;
    }
    if(img.cols != img_width || img.rows != img_height)
        cv::resize(img, img, cv::Size(img_width, img_height), (0, 0), (0, 0), cv::INTER_LINEAR);

    // Load model
    FILE *fp = fopen(model_path, "rb");
    if(fp == NULL) {
        printf("fopen %s fail!\n", model_path);
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    int model_len = ftell(fp);
    void *model = malloc(model_len);
    fseek(fp, 0, SEEK_SET);
    if(model_len != fread(model, 1, model_len, fp)) {
        printf("fread %s fail!\n", model_path);
        free(model);
        return -1;
    }

    // Start Inference
    rknn_input inputs[1];
    rknn_output outputs[2];
    rknn_tensor_attr outputs_attr[2];

    int ret = 0;
    rknn_context ctx = 0;

    ret = rknn_init(&ctx, model, model_len, RKNN_FLAG_PRIOR_MEDIUM);
    if(ret < 0) {
        printf("rknn_init fail! ret=%d\n", ret);
        goto Error;
    }

    outputs_attr[0].index = 0;
    ret = rknn_query(ctx, RKNN_QUERY_OUTPUT_ATTR, &(outputs_attr[0]), sizeof(outputs_attr[0]));
    if(ret < 0) {
        printf("rknn_query fail! ret=%d\n", ret);
        goto Error;
    }

    outputs_attr[1].index = 1;
    ret = rknn_query(ctx, RKNN_QUERY_OUTPUT_ATTR, &(outputs_attr[1]), sizeof(outputs_attr[1]));
    if(ret < 0) {
        printf("rknn_query fail! ret=%d\n", ret);
        goto Error;
    }

    inputs[0].index = input_index;
    inputs[0].buf = img.data;
    inputs[0].size = img_width * img_height * img_channels;
    inputs[0].pass_through = false;
    inputs[0].type = RKNN_TENSOR_UINT8;
    inputs[0].fmt = RKNN_TENSOR_NHWC;
    ret = rknn_inputs_set(ctx, 1, inputs);
    if(ret < 0) {
        printf("rknn_input_set fail! ret=%d\n", ret);
        goto Error;
    }

    ret = rknn_run(ctx, nullptr);
    if(ret < 0) {
        printf("rknn_run fail! ret=%d\n", ret);
        goto Error;
    }

    outputs[0].want_float = true;
    outputs[0].is_prealloc = false;
    outputs[1].want_float = true;
    outputs[1].is_prealloc = false;
    ret = rknn_outputs_get(ctx, 2, outputs, nullptr);
    if(ret < 0) {
        printf("rknn_outputs_get fail! ret=%d\n", ret);
        goto Error;
    }

    // Process output
    if(outputs[0].size == outputs_attr[0].n_elems*sizeof(float) && outputs[1].size == outputs_attr[1].n_elems*sizeof(float))
    {
        float boxPriors[4][NUM_RESULTS];
        string labels[91];

        /* load label and boxPriors */
        loadLabelName(label_path, labels);
        loadCoderOptions(box_priors_path, boxPriors);

        float* predictions = (float*)outputs[0].buf;
        float* outputClasses = (float*)outputs[1].buf;

        int output[2][NUM_RESULTS];

        /* transform */
        decodeCenterSizeBoxes(predictions, boxPriors);

        int validCount = scaleToInputSize(outputClasses, output, NUM_CLASSES);
        printf("validCount: %d\n", validCount);

        if (validCount < 100) {
            /* detect nest box */
            nms(validCount, predictions, output);

            Mat rgba = imread(img_path, CV_LOAD_IMAGE_UNCHANGED);
            cv::resize(rgba, rgba, cv::Size(1200, 1200), (0, 0), (0, 0), cv::INTER_LINEAR);

            /* box valid detect target */
            for (int i = 0; i < validCount; ++i) {
                if (output[0][i] == -1) {
                    continue;
                }
                int n = output[0][i];
                int topClassScoreIndex = output[1][i];

                int x1 = static_cast<int>(predictions[n * 4 + 1] * rgba.cols);
                int y1 = static_cast<int>(predictions[n * 4 + 0] * rgba.rows);
                int x2 = static_cast<int>(predictions[n * 4 + 3] * rgba.cols);
                int y2 = static_cast<int>(predictions[n * 4 + 2] * rgba.rows);

                string label = labels[topClassScoreIndex];

                std::cout << label << "\t@ (" << x1 << ", " << y1 << ") (" << x2 << ", " << y2 << ")" << "\n";

                rectangle(rgba, Point(x1, y1), Point(x2, y2), colorArray[topClassScoreIndex%10], 3);
                putText(rgba, label, Point(x1, y1 - 12), 1, 2, Scalar(0, 255, 0, 255));
            }
            imwrite("out.jpg", rgba);
            printf("write out.jpg succ!\n");
        } else {
            printf("validCount too much!\n");
        }
    }
    else
    {
        printf("rknn_outputs_get fail! get outputs_size = [%d, %d], but expect [%lu, %lu]!\n",
            outputs[0].size, outputs[1].size, outputs_attr[0].n_elems*sizeof(float), outputs_attr[1].n_elems*sizeof(float));
    }

    rknn_outputs_release(ctx, 2, outputs);

Error:
    if(ctx)             rknn_destroy(ctx);
    if(model)           free(model);
    if(fp)              fclose(fp);
    return 0;
}