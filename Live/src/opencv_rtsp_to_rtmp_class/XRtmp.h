#pragma once

class AVCodecContext;
class AVPacket;

class XRtmp
{
public:
	static XRtmp* Get(unsigned char index = 0);

	virtual bool Init(const char* url) = 0;

	virtual bool AddStream(const AVCodecContext *c) = 0;

	virtual bool SendHeader() = 0;

	virtual bool SendFrame(AVPacket *pkt) = 0;

	virtual ~XRtmp();

protected:
	XRtmp();
};

