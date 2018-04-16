#include "classifier.hpp"

/* Pair (label, confidence) representing a prediction. */


Classifier::Classifier(const string& model_file,
                       const string& trained_file,
                       const string& mean_file,
                       bool gpu_mode) {
  if (gpu_mode)
    Caffe::set_mode(Caffe::GPU);
  else
    Caffe::set_mode(Caffe::CPU);
    
  /* Load the network. */
  std::cout<<"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"<<std::endl;
  net_.reset(new Net<float>(model_file, TEST));
  std::cout<<"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"<<std::endl;
  net_->CopyTrainedLayersFrom(trained_file);
  std::cout<<"cccccccccccccccccccccccccc"<<std::endl;

  CHECK_EQ(net_->num_inputs(), 1) << "Network should have exactly one input.";
  CHECK_EQ(net_->num_outputs(), 1) << "Network should have exactly one output.";

  Blob<float>* input_layer = net_->input_blobs()[0];
  num_channels_ = input_layer->channels();
  CHECK(num_channels_ == 3 || num_channels_ == 1)
    << "Input layer should have 1 or 3 channels.";
  input_geometry_ = cv::Size(input_layer->width(), input_layer->height());

  /* Load the binaryproto mean file. */
  SetMean(mean_file);

  Blob<float>* output_layer = net_->output_blobs()[0];
  /* Load labels. */
/*
  std::ifstream labels(label_file.c_str());
  CHECK(labels) << "Unable to open labels file " << label_file;
  string line;
  while (std::getline(labels, line))
    labels_.push_back(string(line));

  CHECK_EQ(labels_.size(), output_layer->channels())
    << "Number of labels is different from the output layer dimension.";
*/
}

static bool PairCompare(const std::pair<float, int>& lhs,
                        const std::pair<float, int>& rhs) {
  return lhs.first > rhs.first;
}

static std::vector<int> Argmax(const std::vector<float>& v, int N) {
  std::vector<std::pair<float, int> > pairs;
  for (size_t i = 0; i < v.size(); ++i)
    pairs.push_back(std::make_pair(v[i], i));
  std::partial_sort(pairs.begin(), pairs.begin() + N, pairs.end(), PairCompare);

  std::vector<int> result;
  for (int i = 0; i < N; ++i)
    result.push_back(pairs[i].second);
  return result;
}

int Classifier::Classify(const cv::Mat& img, int N) {
  std::vector<float> output = Predict(img);

  //N = std::min<int>(labels_.size(), N);
  std::cout << output[0] << " " << output[1] << " "<< output[2] << " " << output[3] << " " << output[4] << " " << output[5] <<" " << output[6] <<" " << output[7] <<std::endl;
  std::vector<int> maxN = Argmax(output, N);
  qLog(LOGLEVEL_VERY_HIGH, "Classifier", "Error", QString("classification output %1 %2 %3 %4 %5 %6 %7 %8").arg(output[0]).arg(output[1]).arg(output[2]).arg(output[3]).arg(output[4]).arg(output[5]).arg(output[6]).arg(output[7]));
  if (output[maxN[0]] < 0.8)
  {
	return 7;
  }
  /*

  if (maxN[0]==6 || maxN[0]==3)
  {
	  if (output[maxN[0]] < 0.8)
		  return 7;
  }
  else if (output[maxN[0]] < 0.95)
  {
	return 7;
  }
  */
  return maxN[0];
}

/* Load the mean file in binaryproto format. */
void Classifier::SetMean(const string& mean_file) {
  BlobProto blob_proto;
  ReadProtoFromBinaryFileOrDie(mean_file.c_str(), &blob_proto);

  /* Convert from BlobProto to Blob<float> */
  Blob<float> mean_blob;
  mean_blob.FromProto(blob_proto);
  CHECK_EQ(mean_blob.channels(), num_channels_)
    << "Number of channels of mean file doesn't match input layer.";

  /* The format of the mean file is planar 32-bit float BGR or grayscale. */
  std::vector<cv::Mat> channels;
  float* data = mean_blob.mutable_cpu_data();
  for (int i = 0; i < num_channels_; ++i) {
    /* Extract an individual channel. */
    cv::Mat channel(mean_blob.height(), mean_blob.width(), CV_32FC1, data);
    channels.push_back(channel);
    data += mean_blob.height() * mean_blob.width();
  }

  /* Merge the separate channels into a single image. */
  cv::Mat mean;
  cv::merge(channels, mean);

  /* Compute the global mean pixel value and create a mean image
   * filled with this value. */
  cv::Scalar channel_mean = cv::mean(mean);
  mean_ = cv::Mat(input_geometry_, mean.type(), channel_mean);
}

std::vector<float> Classifier::Predict(const cv::Mat& img) {
  Blob<float>* input_layer = net_->input_blobs()[0];
  input_layer->Reshape(1, num_channels_,
                       input_geometry_.height, input_geometry_.width);
  /* Forward dimension change to all layers. */
  net_->Reshape();

  std::vector<cv::Mat> input_channels;
  WrapInputLayer(&input_channels);

  Preprocess(img, &input_channels);

  net_->Forward();

  /* Copy the output layer to a std::vector */
  //Blob<float>* output_layer = net_->blob_by_name("pool4").get();//net_->output_blobs()[0];
  Blob<float>* output_layer = net_->output_blobs()[0];
  const float* begin = output_layer->cpu_data();
  const float* end = begin + output_layer->channels();
  return std::vector<float>(begin, end);
}

/* Wrap the input layer of the network in separate cv::Mat objects
 * (one per channel). This way we save one memcpy operation and we
 * don't need to rely on cudaMemcpy2D. The last preprocessing
 * operation will write the separate channels directly to the input
 * layer. */
void Classifier::WrapInputLayer(std::vector<cv::Mat>* input_channels) {
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

void Classifier::Preprocess(const cv::Mat& img,
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
  if (sample.size() != input_geometry_)
    cv::resize(sample, sample_resized, input_geometry_);
  else
    sample_resized = sample;

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

  CHECK(reinterpret_cast<float*>(input_channels->at(0).data)
        == net_->input_blobs()[0]->cpu_data())
    << "Input channels are not wrapping the input layer of the network.";
}


int Classifier::ClassifyAugmentedWithImage(cv::Mat img, cv::Rect rect){
  cv::Mat resized_img;
  cv::Size size = img.size();
  //resize(img, resized_img, cv::Size(size.width/4, size.height/4));
  int c_h = 3;
  int c_v = 3;

  int cls_0_count = 0;
  int cls_1_count = 0;
  int cls_2_count = 0;
  int cls_3_count = 0;
  int cls_4_count = 0;
  int cls_5_count = 0;
  int cls_6_count = 0;
  int cls_7_count = 0; //etc
  for (int i=0; i<c_h; i++)
  {
	for (int j=0; j<c_v; j++)
    {
	  int new_x = std::max(0,rect.x-(c_h/2-i)*5);
	  int new_y = std::max(0, rect.y-(c_v/2-j)*5);
      cv::Rect roi(new_x, new_y, std::min(size.width-new_x, rect.width), std::min(size.height-new_y, rect.height));
      cv::Mat cropped_img = img(roi);
	  /*
	  char path[100];
  	  const clock_t t = clock();
	  snprintf(path, sizeof(path), "./patch/%ld_%d_%d.jpg",t,i,j);
	  string path_str = path;
	  imwrite(path,cropped_img);
	  */
	  cv::Mat resized_crop;
	  resize(cropped_img, resized_crop, cv::Size(224,224));
      
      int p = Classify(resized_crop);
  
      if (p== 0) 	cls_0_count+=1;
      else if (p== 1) 	cls_1_count+=1;
      else if (p== 2) 	cls_2_count+=1;
      else if (p== 3) 	cls_3_count+=1;
      else if (p== 4) 	cls_4_count+=1;
      else if (p== 5) 	cls_5_count+=1;
      else if (p== 6) 	cls_6_count+=1;
      else if (p== 7) 	cls_7_count+=1;
    }
  }
  std::cout << cls_0_count << " "<< cls_1_count << " " << cls_2_count << " " << cls_3_count << " " << cls_4_count << " " << cls_5_count << " " << cls_6_count << " " << cls_7_count << std::endl;
  int cls_count[8] = {cls_0_count, cls_1_count, cls_2_count, cls_3_count, cls_4_count, cls_5_count, cls_6_count, cls_7_count};
  const int N = sizeof(cls_count)/ sizeof(int);
  int fin_cls = std::distance(cls_count, std::max_element(cls_count, cls_count+N));
  return fin_cls;
}

int Classifier::ClassifyAugmentedWithPath(string file, cv::Rect rect)
{
  cv::Mat img = cv::imread(file, -1);
  CHECK(!img.empty()) << "Unable to decode image " << file;
  return ClassifyAugmentedWithImage(img, rect);
}
