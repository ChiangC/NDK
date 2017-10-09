#pragma once
#include "XData.h"
#include "XDataThread.h"
#include "XFilter.h"
#include <vector>

class XVideoCapture :public XDataThread
{
public:
	int width = 0;
	int height = 0;
	int fps = 0;

	static XVideoCapture *Get(unsigned char index = 0);

	virtual bool Init(int camIndex = 0) = 0;
	
	virtual bool Init(const char *url) = 0;

	virtual void Stop() = 0;

    void AddFilter(XFilter *filter)
    {
        fmutext.lock();
        filters.push_back(filter);
        fmutext.unlock();
    }
	virtual ~XVideoCapture();

protected:
    QMutex fmutext;
    std::vector<XFilter*> filters;
	XVideoCapture();
};

