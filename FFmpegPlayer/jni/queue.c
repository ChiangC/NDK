
struct Queue{
	//长度
	int size;

	//任意类型的指针数组,这里每一个元素都是AVPacket指针
//	AVPacket **packets;
	void** tab;

	int next_to_write;
	int next_to_read;


};
