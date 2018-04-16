#ifndef CAMERAFRAMEGRABBER_H
#define CAMERAFRAMEGRABBER_H

#include <QAbstractVideoSurface>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/video/background_segm.hpp>

#include <QTime>
#include <iostream>
using namespace std;

class CameraFrameGrabber : public QAbstractVideoSurface
{
    Q_OBJECT
    public:
        explicit CameraFrameGrabber(QObject *parent = 0);

        QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const;

        bool present(const QVideoFrame &frame);
private:
        cv::Mat QImageToCvMat( const QImage &inImage);
        cv::Mat prev_frame;
        cv::Mat next_frame;
        cv::Mat current_frame;
        cv::Mat motion_history;
        cv::Mat mg_mask;
        cv::Mat mg_orient;
        cv::Mat seg_mask;
        vector<cv::Rect> seg_bounds;
        cv::Ptr< cv::BackgroundSubtractor> pMOG;
        cv::Mat element;
        cv::Mat fgMaskMOG;

        cv::VideoWriter writer;

        QTime timeCheck;

        uint mode;

        int detectMotion(const cv::Mat & motion, cv::Mat & result, cv::Mat & result_cropped,
                         int x_start, int x_stop, int y_start, int y_stop,
                         int max_deviation,
                         cv::Scalar & color);
    signals:
        void frameAvailable(QImage frame);

    public slots:

};

#endif // CAMERAFRAMEGRABBER_H
