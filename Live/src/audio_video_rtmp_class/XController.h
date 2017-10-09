#pragma once
#include <QThread>
#include <string>


class XController:protected QThread
{
public:
    std::string outUrl;
    int cameraIndex = -1;
    std:string inUrl = "";


    static XController *Get()
    {
        static XController xc;
        return &xc;
    }

    //设定美颜参数
    virtual bool Set(std::string key, double val);
    virtual bool Start();
    virtual void Stop();

    virtual ~XController();

protected:
    XController();

};
