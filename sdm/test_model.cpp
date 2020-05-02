#include <vector>
#include <iostream>
#include <fstream>
#include "mropencv.h"
#include "ldmarkmodel.h"

using namespace std;
using namespace cv;

int main()
{
    ldmarkmodel modelt;
    std::string modelFilePath = "model/roboman-landmark-model.bin";
    if(!load_ldmarkmodel(modelFilePath, modelt)){
        std::cout << "failed to load sdm model." << std::endl;
        exit(-1);
    }

    cv::VideoCapture mCamera(0);
    if(!mCamera.isOpened()){
        std::cout << "Camera opening failed..." << std::endl;
        system("pause");
        return 0;
    }
    cv::Mat Image;
    cv::Mat current_shape;
    for(;;){
        mCamera >> Image;
        modelt.track(Image, current_shape);
        cv::Vec3d eav;
//        modelt.EstimateHeadPose(current_shape, eav);
        modelt.drawPose(Image, current_shape, 50);

        int numLandmarks = current_shape.cols/2;
        for(int j=0; j<numLandmarks; j++){
            int x = current_shape.at<float>(j);
            int y = current_shape.at<float>(j + numLandmarks);
            std::stringstream ss;
            ss << j;
//            cv::putText(Image, ss.str(), cv::Point(x, y), 0.5, 0.5, cv::Scalar(0, 0, 255));
            cv::circle(Image, cv::Point(x, y), 2, cv::Scalar(0, 0, 255), -1);
        }
        cv::imshow("img", Image);
        if(27 == cv::waitKey(1)){
            mCamera.release();
            cv::destroyAllWindows();
            break;
        }
    }
    return 0;
}