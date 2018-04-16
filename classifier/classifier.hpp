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

#include "worker.h"
#include "zmsg.h"
#include "publisher.h"

using namespace caffe;  // NOLINT(build/namespaces)
using std::string;

class Classifier {
 public:
  Classifier(){}
  Classifier(const string& model_file,
             const string& trained_file,
             const string& mean_file,
             bool gpu_mode);

  int Classify(const cv::Mat& img, int N = 5);
  int ClassifyAugmentedWithPath(string file, cv::Rect rect);
  int ClassifyAugmentedWithImage(cv::Mat img, cv::Rect rect);

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

//Classifier get_classifier(string model_path, string network_path, string mean_file_path, string synset_path, bool gpu_mode);
