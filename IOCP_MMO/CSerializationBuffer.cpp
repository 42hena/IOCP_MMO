#include "CSerializationBuffer.h"

// ##################################################
// #            Default Constructor                 #
// ##################################################

CSerialization::CSerialization()
	: iBufferSize(e_BUFFER_DEFAULT)
{
	buffer = new char[iBufferSize];
	pPush = pPop = buffer;
}

// ##################################################
// #                   Constructor                  #
// ##################################################

CSerialization::CSerialization(int p_iBufferSize)
	: iBufferSize(p_iBufferSize)
{
	buffer = new char[iBufferSize];
	pPush = pPop = buffer;
}

// ##################################################
// #                   Desstructor                  #
// ##################################################

CSerialization::~CSerialization()
{
	delete[] buffer;
}

// ##################################################
// #                   Utils                        #
// ##################################################

void	CSerialization::Clear(void)
{
	pPush = pPop = buffer;
}

int		CSerialization::MoveWritePos(int iSize)
{
	if (iSize <= 0)
		return 0;
	pPush += iSize;
	return iSize;
}

int		CSerialization::MoveReadPos(int iSize)
{
	if (iSize <= 0)
		return 0;
	pPop += iSize;
	return iSize;
}

int		CSerialization::GetData(char* chpDest, int iSrcSize)
{
	/*iSrcSize = pop_pos > iSrcSize ? iSrcSize : pop_pos;
	for (int i = pop_pos; i < iSrcSize + pop_pos; ++i)
		chpDest[i - pop_pos] = buffer[i];*/
	return iSrcSize;
}

int		CSerialization::PutData(char* chpSrc, int iSrcSize)
{
	/*iSrcSize = (iBufferSize - push_pos) > iSrcSize ? iSrcSize : (iBufferSize - push_pos);
	for (int i = push_pos; i < iSrcSize + push_pos; ++i)
		buffer[i] = chpSrc[i - push_pos];*/
	return iSrcSize;

}


// ¿¬»êÀÚ ¿À¹ö·Îµù

CSerialization& CSerialization::operator << (bool value)
{
	*(bool*)pPush = value;
	pPush += sizeof(bool);

	return (*this);
}

CSerialization& CSerialization::operator << (char value)
{
	*(char*)pPush = value;
	pPush += sizeof(char);

	return (*this);
}

CSerialization& CSerialization::operator << (unsigned char value)
{
	*(unsigned char*)pPush = value;
	pPush += sizeof(unsigned char);

	return (*this);
}


CSerialization& CSerialization::operator << (short value)
{
	*(short*)pPush = value;
	pPush += sizeof(short);

	return (*this);
}

CSerialization& CSerialization::operator << (unsigned short value)
{
	*(unsigned short*)pPush = value;
	pPush += sizeof(unsigned short);

	return (*this);
}


CSerialization& CSerialization::operator << (int value)
{
	*(int*)pPush = value;
	pPush += sizeof(int);

	return (*this);
}

CSerialization& CSerialization::operator << (unsigned int value)
{
	*(unsigned int*)pPush = value;
	pPush += sizeof(unsigned int);

	return (*this);
}

CSerialization& CSerialization::operator << (long value)
{
	*(long*)pPush = value;
	pPush += sizeof(long);

	return (*this);
}

CSerialization& CSerialization::operator << (unsigned long value)
{
	*(unsigned long*)pPush = value;
	pPush += sizeof(unsigned long);

	return (*this);
}


CSerialization& CSerialization::operator << (__int64 value)
{
	*(__int64*)pPush = value;
	pPush += sizeof(__int64);

	return (*this);
}

CSerialization& CSerialization::operator << (unsigned __int64 value)
{
	*(unsigned __int64*)pPush = value;
	pPush += sizeof(unsigned __int64);

	return (*this);
}

CSerialization& CSerialization::operator << (float value)
{
	*(float*)pPush = value;
	pPush += sizeof(float);

	return (*this);
}

CSerialization& CSerialization::operator << (double value)
{
	*(double*)pPush = value;
	pPush += sizeof(double);

	return (*this);
}


// Ãâ·Â
CSerialization& CSerialization::operator >> (bool& value)
{
	value = *(bool*)(pPop);
	pPop += sizeof(bool);

	return (*this);
}


CSerialization& CSerialization::operator >> (char& value)
{
	value = *(char*)(pPop);
	pPop += sizeof(char);

	return (*this);
}

CSerialization& CSerialization::operator >> (unsigned char& value)
{
	value = *(unsigned char*)(pPop);
	pPop += sizeof(unsigned char);
	return (*this);
}


CSerialization& CSerialization::operator >> (short& value)
{
	value = *(short*)(pPop);
	pPop += sizeof(short);

	return (*this);
}

CSerialization& CSerialization::operator >> (unsigned short& value)
{
	value = *(unsigned short*)(pPop);
	pPop += sizeof(unsigned short);

	return (*this);
}

CSerialization& CSerialization::operator >> (int& value)
{
	value = *(int*)(pPop);
	pPop += sizeof(int);

	return (*this);
}

CSerialization& CSerialization::operator >> (unsigned int& value)
{
	value = *(unsigned int*)(pPop);
	pPop += sizeof(unsigned int);

	return (*this);
}

CSerialization& CSerialization::operator >> (long& value)
{
	value = *(long*)(pPop);
	pPop += sizeof(long);

	return (*this);
}

CSerialization& CSerialization::operator >> (unsigned long& value)
{
	value = *(unsigned long*)(pPop);
	pPop += sizeof(unsigned long);

	return (*this);
}

CSerialization& CSerialization::operator >> (__int64& value)
{
	value = *(__int64*)(pPop);
	pPop += sizeof(__int64);

	return (*this);
}

CSerialization& CSerialization::operator >> (unsigned __int64& value)
{
	value = *(unsigned __int64*)(pPop);
	pPop += sizeof(unsigned __int64);

	return (*this);
}

CSerialization& CSerialization::operator >> (float& value)
{
	value = *(float*)(pPop);
	pPop += sizeof(float);

	return (*this);
}

CSerialization& CSerialization::operator >> (double& value)
{
	value = *(double*)(pPop);
	pPop += sizeof(double);

	return (*this);
}