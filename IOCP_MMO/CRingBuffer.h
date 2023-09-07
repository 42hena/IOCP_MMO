#pragma once
class CRingBuffer {
public:
	// 持失切
	CRingBuffer(void);
	CRingBuffer(int iBufferSize);

	// 社瑚切
	~CRingBuffer();

	// Get total buffer size
	int		GetBufferSize(void);

	int		GetUseSize(void);
	int		GetFreeSize(void);

	int		DirectEnqueueSize(void);
	int		DirectDequeueSize(void);

	int		Enqueue(char* chpData, int iSize);
	int		Dequeue(char* chpDest, int iSize);

	int		Peek(char* chpDest, int iSize);

	int		MoveRear(int iSize);
	int		MoveFront(int iSize);
	void	ClearBuffer(void);
	char* GetFrontBufferPtr(void);
	char* GetRearBufferPtr(void);

	char* GetBufferPtr(void)
	{
		return buff;
	}
private:
	int		buffer_size;
	char*	buff;

	int		front_index;
	int		rear_index;
};
