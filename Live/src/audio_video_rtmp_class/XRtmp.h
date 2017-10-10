#pragma once
#include "XData.h"

class AVCodecContext;
class AVPacket;

class XRtmp
{
public:
	static XRtmp* Get(unsigned char index = 0);

	virtual bool Init(const char* url) = 0;

	//Add video or audio stream
	//return stream index when success, -1 when failed.
	virtual int AddStream(const AVCodecContext *c) = 0;

	virtual bool SendHeader() = 0;

	virtual bool SendFrame(XData pkt, int streamIndex = 0) = 0;

    virtual void Close() = 0;

	virtual ~XRtmp();

protected:
	XRtmp();
};

