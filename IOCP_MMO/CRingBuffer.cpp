#include <memory.h>
#include <Windows.h>
#include "CRingBuffer.h"


// ##################################################
// #                  Constructor                   #
// ##################################################

CRingBuffer::CRingBuffer(void)
	: buffer_size(10000 + 1),
	front_index(0), rear_index(0)
{
	buff = new char[buffer_size];
	// 예외를 던져야 할까?
}

CRingBuffer::CRingBuffer(int iBufferSize)
	: buffer_size(iBufferSize + 1),
	front_index(0), rear_index(0)
{
	buff = new char[buffer_size];
}

// ##################################################
// #                   Destructor                   #
// ##################################################

CRingBuffer::~CRingBuffer()
{
	delete[] buff;
}

// ------------------------------------------------------

// ##################################################
// #          Buffer의 총 크기를 얻는 함수          #
// ##################################################

int		CRingBuffer::GetBufferSize(void)
{
	return (buffer_size);
}

// ##################################################
// #          Buffer에서 현재 사용되고 있는 크기    #
// ##################################################
//-
int		CRingBuffer::GetUseSize(void)
{
	int temp_rear_index = rear_index;
	int temp_front_index = front_index;
	if (temp_rear_index >= temp_front_index)
		return temp_rear_index - temp_front_index;
	else
		return buffer_size + (temp_rear_index - temp_front_index);
}

// ##################################################
// #          Buffer에서 남아있는 크기              #
// ##################################################
//-
int		CRingBuffer::GetFreeSize(void)
{
	return (buffer_size - 1 - GetUseSize());
}

void	CRingBuffer::ClearBuffer(void)
{
	front_index = rear_index = 0;
}

int	CRingBuffer::DirectEnqueueSize(void)
{
	int temp_rear_index = rear_index;
	int temp_front_index = front_index;

	if (temp_front_index <= temp_rear_index)
	{
		if (temp_front_index == 0)
			return buffer_size - temp_rear_index - 1;
		else
			return buffer_size - temp_rear_index;
	}
	else
		return temp_front_index - 1 - temp_rear_index;
}

int	CRingBuffer::DirectDequeueSize(void)
{
	int temp_front_index = front_index;
	int temp_rear_index = rear_index;

	if (temp_front_index <= temp_rear_index)
		return temp_rear_index - temp_front_index;
	else
		return buffer_size - temp_front_index;
}

char* CRingBuffer::GetFrontBufferPtr(void)
{
	return buff + front_index;
}

char* CRingBuffer::GetRearBufferPtr(void)
{
	return buff + rear_index;
}


int		CRingBuffer::MoveRear(int iSize)
{
	if (GetFreeSize() < iSize)
		return 0;
	int temp_rear_index = rear_index;

	temp_rear_index = (temp_rear_index + iSize) % buffer_size;
	rear_index = temp_rear_index;
	return iSize;
}

int	CRingBuffer::MoveFront(int iSize)
{
	if (GetUseSize() < iSize)
		return 0;
	int temp_front_index = front_index;

	temp_front_index = (temp_front_index + iSize) % buffer_size;
	front_index = temp_front_index;
	return iSize;
}

int		CRingBuffer::Enqueue(char* chpData, int iSize)
{
	// 용량 체크
	if (iSize > GetFreeSize())
		return 0;
	int eqSize = DirectEnqueueSize();
	int temp_rear_index = rear_index;
	char* pRear = buff + temp_rear_index;
	if (eqSize < iSize)
	{
		memcpy(pRear, chpData, eqSize);
		memcpy(buff, chpData + eqSize, iSize - eqSize);
	}
	else
	{
		memcpy(pRear, chpData, iSize);
	}
	temp_rear_index = (temp_rear_index + iSize) % buffer_size;
	rear_index = temp_rear_index;
	return iSize;
}

int		CRingBuffer::Dequeue(char* chpDest, int iSize)
{
	int useSize = GetUseSize();
	if (iSize > useSize)
		return 0;
	// 용량 체크

	int dqSize = DirectDequeueSize();
	int temp_front_index = front_index;
	char* pFront = GetFrontBufferPtr();
	iSize = (useSize < iSize) ? useSize : iSize;

	if (dqSize < iSize)
	{
		memcpy(chpDest, pFront, dqSize);
		memcpy(chpDest + dqSize, buff, iSize - dqSize);
	}
	else
	{
		memcpy(chpDest, pFront, iSize);
	}

	temp_front_index = (temp_front_index + iSize) % buffer_size;
	front_index = temp_front_index;
	return iSize;
}

int		CRingBuffer::Peek(char* chpDest, int iSize)
{
	int useSize = GetUseSize();
	if (iSize > useSize)
		return 0;
	int dqSize = DirectDequeueSize();
	char* pFront = GetFrontBufferPtr();
	// 용량 체크
	iSize = (useSize < iSize) ? useSize : iSize;
	if (dqSize < iSize)
	{
		memcpy(chpDest, pFront, dqSize);
		memcpy(chpDest + dqSize, buff, iSize - dqSize);
	}
	else
	{
		memcpy(chpDest, pFront, iSize);
	}
	return (iSize);
}