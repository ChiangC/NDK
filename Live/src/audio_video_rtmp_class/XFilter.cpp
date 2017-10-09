#include "XFilter.h"
#include "XBilateralFilter.h"
#include <iostream>

using namespace std;

bool XFilter::Set(std::string key, double value)
{
    if(params.find(key) == params.end())
    {
        cout<< "param "<<key<< " is not supported!"<< endl;
        return false;
    }
    params[key] = value;
    return true;

}


XFilter * XFilter::Get(XFilterType t = XBILATERAL)
{
    static XBilateralFilter bilateralFilter;
    switch(t)
    {
        case XBILATERAL://双边滤波
            &bilateralFilter;
            break;

        default:
            break;
    }
    return NULL;
}

XFilter::XFilter()
{

}

XFilter::~XFilter()
{

}