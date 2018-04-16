#include <caffe/caffe.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <algorithm>
#include <iosfwd>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <sys/time.h>
#include <ctime>
#include <time.h>
#include <stdio.h>

using namespace caffe;  // NOLINT(build/namespaces)
using std::string;

class Detector {
 public:
  Detector(){}
  Detector(const string& att_model_file,
             const string& att_trained_file,
             bool gpu_mode);

  cv::Rect DetectBbox(const cv::Mat& img);

 private:
  void SetMean(const string& mean_file);

  std::vector<float> Predict(const cv::Mat& img);

  void WrapInputLayer(std::vector<cv::Mat>* input_channels);

  void Preprocess(const cv::Mat& img,
                  std::vector<cv::Mat>* input_channels);

 private:
  shared_ptr<Net<float> > net_;
  cv::Size input_geometry_;
  int num_channels_;
  cv::Mat mean_;
};
