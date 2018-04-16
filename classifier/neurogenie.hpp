#include "detector.hpp"
#include "classifier.hpp"
//#include <QObject>
//#include <QCoreApplication>
#include "worker.h"
#include "zmsg.h"
#include "publisher.h"
class Neurogenie {
	public:
		Neurogenie(Detector detector, Classifier classifier);
		int DetectAndClassifyWithImage(cv::Mat img);
		int DetectAndClassifyWithPath(string path);
	private:
		Detector dt_;
		Classifier cls_;
};
