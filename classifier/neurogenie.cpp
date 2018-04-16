#include "neurogenie.hpp"

//Neurogenie::Neurogenie(Detector::Detector detector, Classifier::Classifier classifier){
Neurogenie::Neurogenie(Detector detector, Classifier classifier){
	dt_ = detector;
	cls_ = classifier;
}

int Neurogenie::DetectAndClassifyWithImage(cv::Mat img){
	try{
		cv::Rect rect	= dt_.DetectBbox(img);
        qLog(LOGLEVEL_VERY_HIGH, "Detector", "Error", QString("bbox:  x: %1 y: %2 w: %3 h: %4").arg(rect.x).arg(rect.y).arg(rect.width).arg(rect.height));
		if (rect.x<=0 || rect.y<=0 || rect.width<=0 || rect.height<=0)
			return 7;
		int cls 		= cls_.ClassifyAugmentedWithImage(img, rect);
		return cls;
	}
	catch(int e){
		return 7;
	}
}
int Neurogenie::DetectAndClassifyWithPath(string path){
	cv::Mat img = cv::imread(path);
	return DetectAndClassifyWithImage(img);
}
