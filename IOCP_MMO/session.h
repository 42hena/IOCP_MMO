#pragma once
// type Á¤ÀÇ
#define SESSEION_TYPE DWORD
#define SESSION_ITER_TYPE unordered_map<SESSEION_TYPE, st_session *>::iterator

#include <set>

#define SECTOR_SIZE 100
#define dfSECTOR_SIZE 100
#define dfSECTOR_MAX_X 6400 / SECTOR_SIZE
#define dfSECTOR_MAX_Y 6400 / SECTOR_SIZE
#define dfERROR_RANGE 50
struct st_Sector
{
	short sec_y;
	short sec_x;
	bool operator==(const st_Sector& copy) const
	{
		if (sec_y == copy.sec_y && sec_x == copy.sec_x)
			return true;
		return false;
	}
	bool operator!=(const st_Sector& copy) const
	{
		return !(*this == copy);
	}
};

struct st_SECTOR_AROUND
{
	int count;
	st_Sector around[9];
};



struct st_session
{
	st_session()
	{
		InitializeCriticalSection(&session_cs);
	}
	~st_session()
	{
		DeleteCriticalSection(&session_cs);
	}

	SOCKET socket;
	DWORD session_id;
	OVERLAPPED send_overlapped;
	OVERLAPPED recv_overlapped;
	CRingBuffer send_buffer;
	CRingBuffer recv_buffer;
	DWORD IO_count;
	LONG send_flag;
	CRITICAL_SECTION session_cs;
	DWORD		last_recv_time;
};


struct st_Character
{
	st_Character()
		: action(8),
		direction(0),
		x(rand() % 6400),
		y(rand() % 6400),
		hp(100)
	{ 
		sector.sec_y = y / SECTOR_SIZE;
		old_sector.sec_y = sector.sec_y;
		sector.sec_x = x / SECTOR_SIZE;
		old_sector.sec_x = sector.sec_x;
	}

	st_session* session; // 8
	DWORD		character_id;// 4
	BYTE		action; // 1
	BYTE		direction; // 1
	short		x;
	short		y;
	char		hp;

	st_Sector	sector; //4
	st_Sector	old_sector;// 4
};