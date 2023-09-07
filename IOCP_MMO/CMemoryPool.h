#ifndef  __CMEMORY_POOL__
#define  __CMEMORY_POOL__

#include <new.h>
#include <iostream>
#include <Windows.h>

template <class T>
class CMemoryPool
{
private:
	/*
	* ��� ����ü
	*/
	struct st_BLOCK_NODE
	{
		st_BLOCK_NODE()
			:
#ifdef DEBUG
			m_lowerBlock(nullptr),
			m_upperBlock(nullptr),
#endif
			stpNextBlock(nullptr)
		{ }

		// st_BLOCK_NODE's member variable
#ifdef DEBUG
		st_BLOCK_NODE* m_lowerBlock;
#endif
		T				m_data;
		st_BLOCK_NODE* stpNextBlock;
#ifdef DEBUG
		st_BLOCK_NODE* m_upperBlock;
#endif
	};

public:
	/*
	* ������
	* parameters: (int) �ʱ� �� ����.
	*			  (bool) Alloc �� ������ / Free �� �ı��� ȣ�� ����
	* return:     (void)
	*/
	CMemoryPool(int blockNum, bool placementNew = false)
		: m_blockSize(sizeof(st_BLOCK_NODE)),
		m_allocCount(blockNum),
		m_useCount(0),
		m_placementNewFlag(placementNew)
	{
		InitializeSRWLock(&m_memoryPoolLock);
		st_BLOCK_NODE* prevNode = nullptr;
		st_BLOCK_NODE* newNode = nullptr;

		if (m_placementNewFlag == false)
		{
			for (int i = 0; i < blockNum; ++i)
			{
				newNode = new st_BLOCK_NODE;
				//newNode->m_allocFlag = true;
				if (prevNode == nullptr)
				{
					newNode->stpNextBlock = nullptr;
				}
				else
				{
					newNode->stpNextBlock = prevNode;
				}
				prevNode = newNode;
			}
		}
		else
		{
			for (int i = 0; i < blockNum; ++i)
			{
				newNode = (st_BLOCK_NODE*)malloc(m_blockSize);
				//newNode->m_allocFlag = false;

				if (prevNode == nullptr)
				{
					newNode->stpNextBlock = nullptr;
				}
				else
				{
					newNode->stpNextBlock = prevNode;
				}
				prevNode = newNode;
			}
		}
		_pFreeNode = newNode;
	}

	/*
	* �ı���
	* parameters: (void)
	* return:     (void)
	*/
	virtual	~CMemoryPool()
	{
		// TODO: _pFreeNode Ǯ ���� �ڵ� ������ �� ��. m_placementNewFlag�� ���ؼ� ������ �� ��
	}

	/*
	* �� �ϳ��� �Ҵ�޴´�.
	* parameters: (void)
	* return: (T *: ����Ÿ �� ������)
	*/
	T* Alloc(void)
	{
		st_BLOCK_NODE* new_node = nullptr;

		AcquireSRWLockExclusive(&m_memoryPoolLock);
		if (m_allocCount > m_useCount)
		{
			new_node = _pFreeNode;
			_pFreeNode = _pFreeNode->stpNextBlock;
			if (m_placementNewFlag == true)
			{
				// excute placement new
#ifdef DEBUG
				T* tmp_node = new ((char*)new_node + sizeof(void*)) T;
#else
				T* tmp_node = new ((char*)new_node) T;
#endif

				++m_useCount;
				ReleaseSRWLockExclusive(&m_memoryPoolLock);
				return tmp_node;
			}
		}
		else
		{
			if (m_placementNewFlag == false)
			{
				new_node = new st_BLOCK_NODE;
				new_node->stpNextBlock = nullptr;
			}
			else
			{
				st_BLOCK_NODE* pMemoryBlock = (st_BLOCK_NODE*)malloc(m_blockSize);
				pMemoryBlock->stpNextBlock = nullptr;

#ifdef DEBUG
				T* tmp_node = new ((char*)pMemoryBlock + sizeof(void*)) T;
#else
				T* tmp_node = new ((char*)pMemoryBlock) T;
#endif
				++m_useCount;

				ReleaseSRWLockExclusive(&m_memoryPoolLock);
				return tmp_node;
			}
			++m_allocCount;
		}
		++m_useCount;
		ReleaseSRWLockExclusive(&m_memoryPoolLock);
#ifdef DEBUG
		return (T*)((char*)new_node + sizeof(void*));
#else
		return (T*)((char*)new_node);
#endif
	}

	/*
	* ������̴� ���� �����Ѵ�.
	* parameters: (DATA *) �� ������.
	* return: (bool) TRUE, FALSE.
	*/
	bool	Free(T* pData)
	{
		st_BLOCK_NODE* pBlockNode;
		AcquireSRWLockExclusive(&m_memoryPoolLock);

#ifdef DEBUG
		pBlockNode = (st_BLOCK_NODE*)((char*)pData - sizeof(void*));
#else
		pBlockNode = (st_BLOCK_NODE*)((char*)pData);
#endif
		pBlockNode->stpNextBlock = _pFreeNode;
		_pFreeNode = pBlockNode;
		--m_useCount;
		if (m_placementNewFlag == true)
		{
			pData->~T();
		}
		ReleaseSRWLockExclusive(&m_memoryPoolLock);
		return true;
	}

	/*
	* ���� Ȯ�� �� �� ������ ��´�. (�޸�Ǯ ������ ��ü ����)
	* parameters: (void)
	* return:	  (int:  �޸� Ǯ ���� ��ü ����)
	*/
	int		GetAllocCount(void)
	{
		return m_allocCount;
	}

	/*
	* ���� ��� ���� �� ������ ��� �Լ�
	* parameters: (void)
	* return:	  (int: ������� �� ����)
	*/
	int		GetUseCount(void)
	{
		return m_useCount;
	}

	// ���� ������� ��ȯ�� (�̻��) ������Ʈ ���� ����.

private: // member variable
	unsigned long		m_blockSize;
	int					m_allocCount;
	int					m_useCount;
	bool				m_placementNewFlag;
	st_BLOCK_NODE* _pFreeNode;

	SRWLOCK			m_memoryPoolLock;
};

#endif
