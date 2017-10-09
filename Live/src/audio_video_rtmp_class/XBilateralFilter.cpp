#include "XBilateralFilter.h"
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

using namespace cv;
using namespace std;

bool XBilateralFilter::Filter(cv::Mat *src, cv::Mat *dest)
{
    double d = params["d"];
    bilateralFilter(*src, *dest, d, d*&2, d/2);
    return true;
}

XBilateralFilter::XBilateralFilter()
{
    params.insert(make_pair("d", 5));
}

XBilateralFilter::~XBilateralFilter()
{

}
