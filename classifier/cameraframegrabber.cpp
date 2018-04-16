#include "cameraframegrabber.h"

#include <QVideoFrame>
#include <QDebug>
#include <opencv2/video/tracking.hpp>

CameraFrameGrabber::CameraFrameGrabber(QObject *parent) :
    QAbstractVideoSurface(parent),mode(3)
{
    pMOG = cv::createBackgroundSubtractorMOG2();
    element = getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3), cv::Point(1,1) );
    writer.open("/home/superbin/test.avi", CV_FOURCC('D', 'I', 'V', 'X') ,30,cv::Size(1920,1080), true);
}

cv::Mat CameraFrameGrabber::QImageToCvMat( const QImage &inImage )
{
  switch ( inImage.format() )
  {
     // 8-bit, 4 channel
     case QImage::Format_ARGB32:
     case QImage::Format_ARGB32_Premultiplied:
     {
        cv::Mat  mat( inImage.height(), inImage.width(),
                      CV_8UC4,
                      const_cast<uchar*>(inImage.bits()),
                      static_cast<size_t>(inImage.bytesPerLine())
                      );

        return mat.clone() ;
     }

     // 8-bit, 3 channel
     case QImage::Format_RGB32:
     case QImage::Format_RGB888:
     {


        QImage   swapped;

        if ( inImage.format() == QImage::Format_RGB32 )
           swapped = inImage.convertToFormat( QImage::Format_RGB888 );

         swapped = inImage.rgbSwapped();

        return cv::Mat( swapped.height(), swapped.width(),
                        CV_8UC3,
                        const_cast<uchar*>(swapped.bits()),
                        static_cast<size_t>(swapped.bytesPerLine())
                        ).clone();
     }

     // 8-bit, 1 channel
     case QImage::Format_Indexed8:
     {
        cv::Mat  mat( inImage.height(), inImage.width(),
                      CV_8UC1,
                      const_cast<uchar*>(inImage.bits()),
                      static_cast<size_t>(inImage.bytesPerLine())
                      );

        return mat.clone();
     }



     default:
        qWarning() << "ASM::QImageToCvMat() - QImage format not handled in switch:" << inImage.format();
        break;
  }

  return cv::Mat();
}


QList<QVideoFrame::PixelFormat> CameraFrameGrabber::supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const
{
    Q_UNUSED(handleType);
    return QList<QVideoFrame::PixelFormat>()
        << QVideoFrame::Format_ARGB32
        << QVideoFrame::Format_ARGB32_Premultiplied
        << QVideoFrame::Format_RGB32
        << QVideoFrame::Format_RGB24
        << QVideoFrame::Format_RGB565
        << QVideoFrame::Format_RGB555
        << QVideoFrame::Format_ARGB8565_Premultiplied
        << QVideoFrame::Format_BGRA32
        << QVideoFrame::Format_BGRA32_Premultiplied
        << QVideoFrame::Format_BGR32
        << QVideoFrame::Format_BGR24
        << QVideoFrame::Format_BGR565
        << QVideoFrame::Format_BGR555
        << QVideoFrame::Format_BGRA5658_Premultiplied
        << QVideoFrame::Format_AYUV444
        << QVideoFrame::Format_AYUV444_Premultiplied
        << QVideoFrame::Format_YUV444
        << QVideoFrame::Format_YUV420P
        << QVideoFrame::Format_YV12
        << QVideoFrame::Format_UYVY
        << QVideoFrame::Format_YUYV
        << QVideoFrame::Format_NV12
        << QVideoFrame::Format_NV21
        << QVideoFrame::Format_IMC1
        << QVideoFrame::Format_IMC2
        << QVideoFrame::Format_IMC3
        << QVideoFrame::Format_IMC4
        << QVideoFrame::Format_Y8
        << QVideoFrame::Format_Y16
        << QVideoFrame::Format_Jpeg
        << QVideoFrame::Format_CameraRaw
        << QVideoFrame::Format_AdobeDng;
}

int CameraFrameGrabber::detectMotion(const cv::Mat & motion, cv::Mat & result, cv::Mat & result_cropped,
                 int x_start, int x_stop, int y_start, int y_stop,
                 int max_deviation,
                 cv::Scalar & color)
{
    // calculate the standard deviation
    cv::Scalar mean, stddev;
    cv::meanStdDev(motion, mean, stddev);
    // if not to much changes then the motion is real (neglect agressive snow, temporary sunlight)
    if(stddev[0] < max_deviation)
    {
        int number_of_changes = 0;
        int min_x = motion.cols, max_x = 0;
        int min_y = motion.rows, max_y = 0;
        // loop over image and detect changes
        for(int j = y_start; j < y_stop; j+=2){ // height
            for(int i = x_start; i < x_stop; i+=2){ // width
                // check if at pixel (j,i) intensity is equal to 255
                // this means that the pixel is different in the sequence
                // of images (prev_frame, current_frame, next_frame)


                if(motion.at<uchar>(j,i) == 255)
                {
                    number_of_changes++;
                    if(min_x>i) min_x = i;
                    if(max_x<i) max_x = i;
                    if(min_y>j) min_y = j;
                    if(max_y<j) max_y = j;
                }
            }
        }
        if(number_of_changes){
            //check if not out of bounds
            if(min_x-10 > 0) min_x -= 10;
            if(min_y-10 > 0) min_y -= 10;
            if(max_x+10 < result.cols-1) max_x += 10;
            if(max_y+10 < result.rows-1) max_y += 10;
            // draw rectangle round the changed pixel
            cv::Point x(min_x,min_y);
            cv::Point y(max_x,max_y);
            cv::Rect rect(x,y);
            cv::Mat cropped = result(rect);
            cropped.copyTo(result_cropped);
            cv::rectangle(result,rect,color,1);
        }
        return number_of_changes;
    }
    return 0;
}

bool CameraFrameGrabber::present(const QVideoFrame &frame)
{
    if (frame.isValid()) {
        QVideoFrame cloneFrame(frame);
        cloneFrame.map(QAbstractVideoBuffer::ReadOnly);
        const QImage image(cloneFrame.bits(),
                           cloneFrame.width(),
                           cloneFrame.height(),
                           QVideoFrame::imageFormatFromPixelFormat(cloneFrame .pixelFormat()));
        prev_frame = QImageToCvMat(image);
        pMOG->apply(prev_frame, fgMaskMOG);

        imshow("MOG", fgMaskMOG);

        vector<vector<cv::Point >> contours;

        cv::Mat kernel=getStructuringElement(cv::MORPH_ELLIPSE,cv::Size(5,5));
        cv::Mat kernel2=getStructuringElement(cv::MORPH_ELLIPSE,cv::Size(10,10));
        cv::morphologyEx(fgMaskMOG,fgMaskMOG,CV_MOP_ERODE,kernel);
        cv::morphologyEx(fgMaskMOG,fgMaskMOG,CV_MOP_DILATE,kernel2);

        findContours(fgMaskMOG,contours,CV_RETR_TREE,CV_CHAIN_APPROX_SIMPLE);
        //drawContours(prev_frame,contours,-1,cv::Scalar(255,255,255),2);
        //imshow("result",prev_frame);

        vector<vector<cv::Point> > contours_poly( contours.size() );
        vector<cv::Rect> boundRect( contours.size() );
        for( int i = 0; i < contours.size(); i++ )
        {
            //cv::Scalar color = cv::Scalar( 255, 0, 0 );
            cv::approxPolyDP( cv::Mat(contours[i]), contours_poly[i], 3, true );
            boundRect[i] = cv::boundingRect( cv::Mat(contours_poly[i]) );


            //cv::rectangle( prev_frame, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0 );
            // minEnclosingCircle( (Mat)contours_poly[i], center[i], radius[i] );
        }
        writer<< prev_frame;




        //qDebug() << "InComming Frame : (" << cloneFrame.width() << "x" << cloneFrame.height() << ")" << image.format() << "-->" << number_of_changes << ":" << number_of_sequence;
        cloneFrame.unmap();
        return true;
    }
    return false;
}
