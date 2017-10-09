#pragma once
#include <string>
#include <map>

enum XFilterType
{
    XBILATERAL //双边滤波
}

namespace cv
{
    class Mat;
}

class XFilter
{
public :
    static XFilter * XFilter::Get(XFilterType t = XBILATERAL);

    virtual bool Filter(cv::Mat *src, cv::Mat *dest) = 0;

    virtual bool Set(std::string key, double value);

    virtual ~XFilter();

protected:
    std::map<std:string, double> params;
    XFilter();
};
