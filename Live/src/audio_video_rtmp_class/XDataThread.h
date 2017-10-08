#pragma once
#include <QThread>
#include <qmutex.h>
#include "XData.h"
#include <list>

class XDataThread:public QThread
{
public:
	//(�����б��С)�б����ֵ������ɾ����ɵ�����
	int maxList = 100;

	//���б��β����
	virtual void Push(XData d);

	//��ȡ�б������������,���ص�������Ҫ����XData.Drop����
	virtual XData Pop();

	//�����߳�
	virtual bool Start();

	//�˳��̣߳����ȴ��߳��˳�(�����ĺ���)
	virtual void Stop();

	virtual void Clear();

	XDataThread();
	virtual ~XDataThread();

protected:
	//��Ž�������;���ݲ�����ԣ��Ƚ��ȳ�
	std::list<XData> datas;

	//���������б��С
	int dataCount = 0;
	//�������datas
	QMutex mutex;
	//�����߳��˳�
	bool isExit = false;
};

