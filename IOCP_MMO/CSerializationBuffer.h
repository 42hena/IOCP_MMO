#ifndef  __CSerialization_H__
#define  __CSerialization_H__

class CSerialization
{
public:
	enum en_PACKET
	{
		e_BUFFER_DEFAULT = 1400		// 패킷의 기본 버퍼 사이즈.
	};

	// 생성자
	CSerialization();
	CSerialization(int iBufferSize);

	// 상속 대비 + 소멸자
	virtual ~CSerialization();

	// ####################################################################################################
	// #                   Utils                                                                          #
	// ####################################################################################################

		// ##################################################
		// # 제일 처음 상태로 되돌리기                      #
		// #                                                #
		// # Param  : (None)                                # 
		// # return : (None)                                # 
		// ##################################################
	void	Clear(void);

	// ##################################################
	// # 최대 버퍼의 크기                               #
	// #                                                #
	// # Param  : (None)                                # 
	// # return : (int : Max Buffer size)               # 
	// ##################################################
	int	GetBufferSize(void) { return iBufferSize; }

	// ##################################################
	// # 현재 저장 중인 버퍼의 사이즈                   #
	// #                                                #
	// # Param  : (None)                                #
	// # return : (int : using Buffer size)             #
	// ##################################################
	__int64	GetDataSize(void) { return pPush - pPop; }

	// ##################################################
	// # 버퍼의 처음 위치를 반환                        #
	// #                                                #
	// # Param  : (None)                                #
	// # return : (char * : buffer_ptr)                 #
	// ##################################################
	char* GetBufferPtr(void) { return buffer; }

	//////////////////////////////////////////////////////////////////////////
	// 버퍼 Pos 이동. (음수이동은 안됨)
	// GetBufferPtr 함수를 이용하여 외부에서 강제로 버퍼 내용을 수정할 경우 사용. 
	//
	// Parameters: (int) 이동 사이즈.
	// Return: (int) 이동된 사이즈.
	//////////////////////////////////////////////////////////////////////////
	int		MoveWritePos(int iSize);
	int		MoveReadPos(int iSize);


	//////////////////////////////////////////////////////////////////////////
	// 데이타 얻기.
	//
	// Parameters: (char *)Dest 포인터. (int)Size.
	// Return: (int)복사한 사이즈.
	//////////////////////////////////////////////////////////////////////////
	int		GetData(char* chpDest, int iSize);

	//////////////////////////////////////////////////////////////////////////
	// 데이타 삽입.
	//
	// Parameters: (char *)Src 포인터. (int)SrcSize.
	// Return: (int)복사한 사이즈.
	//////////////////////////////////////////////////////////////////////////
	int		PutData(char* chpSrc, int iSrcSize);


	/* ============================================================================= */
	// 연산자 오버로딩
	/* ============================================================================= */
	CSerialization& operator = (CSerialization& clSrcPacket);

	//////////////////////////////////////////////////////////////////////////
	// 넣기.	각 변수 타입마다 모두 만듬.
	//////////////////////////////////////////////////////////////////////////
	CSerialization& operator << (bool);

	CSerialization& operator << (char);
	CSerialization& operator << (unsigned char);

	CSerialization& operator << (short);
	CSerialization& operator << (unsigned short);

	CSerialization& operator << (int);
	CSerialization& operator << (unsigned int);

	CSerialization& operator << (long);
	CSerialization& operator << (unsigned long);

	CSerialization& operator << (__int64);
	CSerialization& operator << (unsigned __int64);

	CSerialization& operator << (float);
	CSerialization& operator << (double);

	//////////////////////////////////////////////////////////////////////////
	// 빼기.	각 변수 타입마다 모두 만듬.
	//////////////////////////////////////////////////////////////////////////
	CSerialization& operator >> (bool&);

	CSerialization& operator >> (char&);
	CSerialization& operator >> (unsigned char&);

	CSerialization& operator >> (short&);
	CSerialization& operator >> (unsigned short&);

	CSerialization& operator >> (int&);
	CSerialization& operator >> (unsigned int&);

	CSerialization& operator >> (long&);
	CSerialization& operator >> (unsigned long&);

	CSerialization& operator >> (__int64&);
	CSerialization& operator >> (unsigned __int64&);

	CSerialization& operator >> (float&);
	CSerialization& operator >> (double&);

protected:
	char* pPush;
	char* pPop;
	char* buffer;
	int	iBufferSize;
};

#endif