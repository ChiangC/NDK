#pragma once
class XData
{
public:
	char *data = 0;
	int size = 0;
	void Drop();
	XData();
	//�����ռ䣬������data����
	XData(char *data, int size);
	virtual ~XData();
};
