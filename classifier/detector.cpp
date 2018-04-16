#include "detector.hpp"

/* Pair (label, confidence) representing a prediction. */


Detector::Detector(const string& model_file,
                   const string& trained_file,
                   bool gpu_mode) {
  if (gpu_mode)
    Caffe::set_mode(Caffe::GPU);
  else
    Caffe::set_mode(Caffe::CPU);
    
  /* Load the network. */
  net_.reset(new Net<float>(model_file, TEST));
  net_->CopyTrainedLayersFrom(trained_file);

  CHECK_EQ(net_->num_inputs(), 1) << "Network should have exactly one input.";
  CHECK_EQ(net_->num_outputs(), 1) << "Network should have exactly one output.";

  Blob<float>* input_layer = net_->input_blobs()[0];
  num_channels_ = input_layer->channels();
  CHECK(num_channels_ == 3 || num_channels_ == 1)
    << "Input layer should have 1 or 3 channels.";
  input_geometry_ = cv::Size(input_layer->width(), input_layer->height());

  /* Load the binaryproto mean file. */
  //SetMean(mean_file);

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

cv::Rect Detector::DetectBbox(const cv::Mat& img) {
  
  cv::Size size = img.size();
  cv::Mat resized;
  resize(img, resized, cv::Size(224,224));

  std::vector<float> output = Predict(resized);

  std::cout << output[0] << " " << output[1] << " "<< output[2] << " " << output[3] << std::endl;
  if (output[0]<-1 || output[1]<-1 || output[2]<0 || output[3]<0){
	return cv::Rect(0,0,0,0);
  }
  output[0]=(output[0]+1)/2;
  output[1]=(output[1]+1)/2;
  std::cout << output[0] << " " << output[1] << " "<< output[2] << " " << output[3] << std::endl;
  cv::Rect bbox_1(	std::min(size.width,	std::max(0, int(size.width *(output[0]-output[2]/2)))),	
		  			std::min(size.height,	std::max(0, int(size.height*(output[1]-output[3]/2)))), 
		  			std::min(size.width-std::max(0, int(size.width *(output[0]-output[2]/2))),	
								std::max(0, int(size.width *(output[2])))),	
					std::min(size.height-std::max(0, int(size.height*(output[1]-output[3]/2))),	
								std::max(0, int(size.height*(output[3])))) );

  cv::Rect bbox_2_input(	std::min(size.width,	std::max(0, int(size.width *(output[0]-output[2]*3/4)))),	
		  					std::min(size.height,	std::max(0, int(size.height*(output[1]-output[3]*3/4)))), 
		  					std::min(size.width-std::max(0, int(size.width *(output[0]-output[2]*3/4))),	
										std::max(0, int(size.width *(output[2]*3/2)))),	
							std::min(size.height-std::max(0, int(size.height*(output[1]-output[3]*3/4))),	
										std::max(0, int(size.height*(output[3]*3/2)))) );
  std::cout<<"bbox_2_input"<<bbox_2_input<<std::endl;
  cv::Mat cropped_2 = img(bbox_2_input);
  cv::Size size_2 = cropped_2.size();
  cv::Mat resized_2;
  resize(cropped_2, resized_2, cv::Size(224,224));

  std::vector<float> output_2 = Predict(resized_2);
  std::cout << output_2[0] << " " << output_2[1] << " "<< output_2[2] << " " << output_2[3] << std::endl;
  output_2[0]=(output_2[0]+1)/2;
  output_2[1]=(output_2[1]+1)/2;
  std::cout << output_2[0] << " " << output_2[1] << " "<< output_2[2] << " " << output_2[3] << std::endl;

  cv::Rect bbox_2(	bbox_2_input.x + std::min(size_2.width,		std::max(0, int(size_2.width *(output_2[0]-output_2[2]/2)))),	
		  			bbox_2_input.y + std::min(size_2.height,	std::max(0, int(size_2.height*(output_2[1]-output_2[3]/2)))), 
		  			std::min(size_2.width-std::max(0, int(size_2.width *(output_2[0]-output_2[2]/2))),	
								std::max(0, int(size_2.width *(output_2[2])))),	
					std::min(size_2.height-std::max(0, int(size_2.height*(output_2[1]-output_2[3]/2))),	
								std::max(0, int(size_2.height*(output_2[3])))) );
  
  cv::Rect bbox_fin(	int((bbox_1.x+bbox_2.x)/2),
						int((bbox_1.y+bbox_2.y)/2),
						int((bbox_1.width+bbox_2.width)/2),
						int((bbox_1.height+bbox_2.height)/2));

  std::cout<<"bbox_fin"<<bbox_fin<<std::endl;


  return bbox_fin;
}

/* Load the mean file in binaryproto format. */
void Detector::SetMean(const string& mean_file) {
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

std::vector<float> Detector::Predict(const cv::Mat& img) {
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
void Detector::WrapInputLayer(std::vector<cv::Mat>* input_channels) {
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

void Detector::Preprocess(const cv::Mat& img,
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
  //cv::subtract(sample_float, mean_, sample_normalized);

  /* This operation will write the separate BGR planes directly to the
   * input layer of the network because it is wrapped by the cv::Mat
   * objects in input_channels. */
  cv::split(sample_float, *input_channels);

  CHECK(reinterpret_cast<float*>(input_channels->at(0).data)
        == net_->input_blobs()[0]->cpu_data())
    << "Input channels are not wrapping the input layer of the network.";
}
