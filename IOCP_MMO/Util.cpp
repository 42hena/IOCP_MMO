#include <list>
#include <unordered_map>
#include <Windows.h>

#include "CRingBuffer.h"
#include "CSerializationBuffer.h"
#include "session.h"
#include "NetworkLibrary.h"
#include "CreateMessage.h"

using namespace std;

extern list< st_Character* > g_sector[dfSECTOR_MAX_Y + 1][dfSECTOR_MAX_X + 1];
extern unordered_map<DWORD, st_Character*>g_character_map;

void SectorProc_AddCharacter(st_Character* character)
{
	short sector_x = character->sector.sec_x;
	short sector_y = character->sector.sec_y;

	g_sector[sector_y][sector_x].push_back(character);
}

void SectorProc_RemoveCharacter(st_Character* character)
{
	short sector_x = character->old_sector.sec_x;
	short sector_y = character->old_sector.sec_y;
	
	g_sector[sector_y][sector_x].remove(character);
}

bool SectorProc_UpdateCharacter(st_Character* character)
{
	// cur_sector update
	character->sector.sec_x = character->x / dfSECTOR_SIZE;
	character->sector.sec_y = character->y / dfSECTOR_SIZE;

	if (character->old_sector.sec_x == character->sector.sec_x &&
		character->old_sector.sec_y == character->sector.sec_y)
		return false;

	// add current sector
	SectorProc_AddCharacter(character);
	// remove current sector
	SectorProc_RemoveCharacter(character);

	return true;
}

void GetSectorAround(int sec_x, int sec_y, st_SECTOR_AROUND* sector_around)
{
	int x, y;

	sector_around->count = 0;

	for (y = -1; y <= 1; ++y)
	{
		if (sec_y + y < 0 || sec_y + y >= dfSECTOR_MAX_Y)
			continue;
		for (x = -1; x <= 1; ++x)
		{
			if (sec_x + x < 0 || sec_x + x >= dfSECTOR_MAX_X)
				continue;
			sector_around->around[sector_around->count].sec_x = sec_x + x;
			sector_around->around[sector_around->count].sec_y = sec_y + y;
			sector_around->count++;
		}
	}
}

void NetworkUtil_SectorOne(int sector_x, int sector_y, CSerialization& buffer, st_Character* exception)
{
	//list<st_Character*>cur_sector = g_sector[sector_y][sector_x];

	st_Character* cur;

	//printf("sector[%d, %d]시작\n", sector_y, sector_x);
	for (auto sector_iter = g_sector[sector_y][sector_x].begin();
		sector_iter != g_sector[sector_y][sector_x].end();
		++sector_iter)
	{
		cur = *sector_iter;
		if (cur != exception)
		{
			//printf("나를 cur->session->session_id[%d]가 session을 추가하려고 시도합니다\n", cur->session->session_id);
			SendPacket(cur->session->session_id, buffer);
			buffer.Clear();
		}
	}
	//printf("sector[%d, %d]끝\n", sector_y, sector_x);
}

// 섹터 메시지
void SectorOne_CreateOthers(int sector_x, int sector_y, st_Character* exception)
{
	//list<st_Character*>cur_sector = g_sector[sector_y][sector_x];
	CSerialization buffer;
	st_Character* cur;

	DWORD id;
	BYTE dir;
	short x;
	short y;
	BYTE hp;

	id = exception->character_id;
	dir = exception->direction;
	x = exception->x;
	y = exception->y;
	hp = exception->hp;


	//printf("sector[%d, %d]시작\n", sector_y, sector_x);
	for (auto sector_iter = g_sector[sector_y][sector_x].begin();
		sector_iter != g_sector[sector_y][sector_x].end();
		++sector_iter)
	{
		cur = *sector_iter;
		if (cur != exception)
		{
			CreateOtherCharacterPacket(buffer, id, dir, x, y, hp);
			SendPacket(cur->session->session_id, buffer);
			buffer.Clear();
		}
	}
	//printf("sector[%d, %d]끝\n", sector_y, sector_x);
}


void SectorOne_DeleteOthers(int sector_x, int sector_y, st_Character* exception)
{
	//list<st_Character*>cur_sector = g_sector[sector_y][sector_x];
	CSerialization buffer;
	st_Character* cur;

	DWORD id;
	
	id = exception->character_id;
	for (auto sector_iter = g_sector[sector_y][sector_x].begin(); sector_iter != g_sector[sector_y][sector_x].end(); ++sector_iter)
	{
		cur = *sector_iter;
		if (cur != exception)
		{
			CreateDeleteCharacterPacket(buffer, id);
			SendPacket(cur->session->session_id, buffer);
			buffer.Clear();
		}
	}
}

// 완료
void SectorOne_MoveStartOthers(int sector_x, int sector_y, st_Character* exception)
{
	// 메시지 변수들
	DWORD id;
	BYTE action;
	short x;
	short y;

	// 전송 버퍼
	CSerialization buffer;
	
	// map 관련 변수
	//list<st_Character*>cur_sector_character_list = g_sector[sector_y][sector_x];
	list<st_Character*>::iterator iter;
	st_Character* cur;

/*
*  code 시작
*/

	id = exception->character_id;
	action = exception->action;
	x = exception->x;
	y = exception->y;

	for (iter = g_sector[sector_y][sector_x].begin(); iter != g_sector[sector_y][sector_x].end(); ++iter)
	{
		cur = *iter;
		if (cur != exception)
		{
			CreateMoveStartPacket(buffer, id, action, x, y);
			SendPacket(cur->session->session_id, buffer);
			buffer.Clear();
		}
	}
}

void SectorOne_MoveStopOthers(int sector_x, int sector_y, st_Character* exception)
{
	// 메시지 변수들
	DWORD id;
	BYTE dir;
	short x;
	short y;

	// 전송 버퍼
	CSerialization buffer;

	// map 관련 변수
	//list<st_Character*>cur_sector_character_list = g_sector[sector_y][sector_x];
	list<st_Character*>::iterator iter;
	st_Character* cur;

	/*
	*  code 시작
	*/

	id = exception->character_id;
	dir = exception->direction;
	x = exception->x;
	y = exception->y;

	for (iter = g_sector[sector_y][sector_x].begin(); iter != g_sector[sector_y][sector_x].end(); ++iter)
	{
		cur = *iter;
		if (cur != exception)
		{
			CreateMoveStopPacket(buffer, id, dir, x, y);
			SendPacket(cur->session->session_id, buffer);
			buffer.Clear();
		}
	}
}

void SectorOne_SyncOthers(int sector_x, int sector_y, st_Character* exception)
{
	// 메시지 변수들
	DWORD id;
	short x;
	short y;

	// 전송 버퍼
	CSerialization buffer;

	// map 관련 변수
	//list<st_Character*>cur_sector_character_list = g_sector[sector_y][sector_x];
	list<st_Character*>::iterator iter;
	st_Character* cur;

	/*
	*  code 시작
	*/

	id = exception->character_id;
	x = exception->x;
	y = exception->y;

	for (iter = g_sector[sector_y][sector_x].begin(); iter != g_sector[sector_y][sector_x].end(); ++iter)
	{
		cur = *iter;
		CreateSyncPacket(buffer, id, x, y);
		SendPacket(cur->session->session_id, buffer);
		buffer.Clear();
	}
	/*CreateSyncPacket(buffer, id, x, y);
	SendPacket(exception->session->session_id, buffer);*/
	buffer.Clear();
}

void SectorOne_Attack1(int sector_x, int sector_y, st_Character* exception)
{
	// 메시지 변수들
	DWORD id;
	BYTE dir;
	short x;
	short y;

	// 전송 버퍼
	CSerialization buffer;

	// map 관련 변수
	//list<st_Character*>cur_sector_character_list = g_sector[sector_y][sector_x];
	list<st_Character*>::iterator iter;
	st_Character* cur;

	/*
	*  code 시작
	*/

	id = exception->character_id;
	dir = exception->direction;
	x = exception->x;
	y = exception->y;

	for (iter = g_sector[sector_y][sector_x].begin(); iter != g_sector[sector_y][sector_x].end(); ++iter)
	{
		cur = *iter;

		if (cur != exception)
		{
			CreateAttack1Packet(buffer, id, dir, x, y);
			SendPacket(cur->session->session_id, buffer);
			buffer.Clear();
		}
	}
}

void SectorOne_Attack2(int sector_x, int sector_y, st_Character* exception)
{
	// 메시지 변수들
	DWORD id;
	BYTE dir;
	short x;
	short y;

	// 전송 버퍼
	CSerialization buffer;

	// map 관련 변수
	//list<st_Character*>cur_sector_character_list = g_sector[sector_y][sector_x];
	list<st_Character*>::iterator iter;
	st_Character* cur;

	/*
	*  code 시작
	*/

	id = exception->character_id;
	dir = exception->direction;
	x = exception->x;
	y = exception->y;

	for (iter = g_sector[sector_y][sector_x].begin(); iter != g_sector[sector_y][sector_x].end(); ++iter)
	{
		cur = *iter;

		if (cur != exception)
		{
			CreateAttack2Packet(buffer, id, dir, x, y);
			SendPacket(cur->session->session_id, buffer);
			buffer.Clear();
		}
	}
}

void SectorOne_Attack3(int sector_x, int sector_y, st_Character* exception)
{
	// 메시지 변수들
	DWORD id;
	BYTE dir;
	short x;
	short y;

	// 전송 버퍼
	CSerialization buffer;

	// map 관련 변수
	//list<st_Character*>cur_sector_character_list = g_sector[sector_y][sector_x];
	list<st_Character*>::iterator iter;
	st_Character* cur;

	/*
	*  code 시작
	*/

	id = exception->character_id;
	dir = exception->direction;
	x = exception->x;
	y = exception->y;

	for (iter = g_sector[sector_y][sector_x].begin(); iter != g_sector[sector_y][sector_x].end(); ++iter)
	{
		cur = *iter;

		if (cur != exception)
		{
			CreateAttack3Packet(buffer, id, dir, x, y);
			SendPacket(cur->session->session_id, buffer);
			buffer.Clear();
		}
	}
}

void SectorOne_Damage(int sector_x, int sector_y, st_Character* exception,
	DWORD vic_id, BYTE vic_hp, DWORD a_id)
{
	//list<st_Character*>cur_sector = g_sector[sector_y][sector_x];
	CSerialization buffer;
	st_Character* cur;

	for (auto sector_iter = g_sector[sector_y][sector_x].begin();
		sector_iter != g_sector[sector_y][sector_x].end();
		++sector_iter)
	{
		cur = *sector_iter;

		if (cur != exception)
		{
			CreateDamagePacket(buffer, a_id, vic_id, vic_hp);
			SendPacket(cur->session->session_id, buffer);
			buffer.Clear();
		}
	}
}

void NetworkUtil_DeleteSectorOneEraseMe(int sector_x, int sector_y, DWORD session_id)
{
	//list<st_Character*>cur_sector = g_sector[sector_y][sector_x];
	CSerialization buffer;
	st_Character* cur;

	//printf("sector[%d, %d]시작\n", sector_y, sector_x);
	for (auto sector_iter = g_sector[sector_y][sector_x].begin();
		sector_iter != g_sector[sector_y][sector_x].end();
		++sector_iter)
	{
		cur = *sector_iter;
		CreateDeleteCharacterPacket(buffer, session_id);
		SendPacket(cur->session->session_id, buffer);
		buffer.Clear();
	}
	//printf("sector[%d, %d]끝\n", sector_y, sector_x);
}

void NetworkUtil_DeleteSectorOneEraseOthers(int sector_x, int sector_y, st_Character* exception)
{
	//list<st_Character*>cur_sector = g_sector[sector_y][sector_x];
	CSerialization buffer;
	st_Character* cur;

	DWORD id;
	BYTE dir;
	short x;
	short y;
	BYTE hp;
	BYTE action;

	id = exception->character_id;
	dir = exception->direction;
	x = exception->x;
	y = exception->y;
	hp = exception->hp;
	action = exception->action;

	//printf("sector[%d, %d]시작\n", sector_y, sector_x);
	for (auto sector_iter = g_sector[sector_y][sector_x].begin();
		sector_iter != g_sector[sector_y][sector_x].end();
		++sector_iter)
	{
		cur = *sector_iter;
		if (cur != exception)
		{
			CreateOtherCharacterPacket(buffer, id, dir, x, y, hp);
			SendPacket(cur->session->session_id, buffer);
			buffer.Clear();
			if (action <= 7)
			{
				CreateMoveStartPacket(buffer, id, action, x, y);
				SendPacket(cur->session->session_id, buffer);
				buffer.Clear();
			}
		}
	}
	//printf("sector[%d, %d]끝\n", sector_y, sector_x);
}


void AddCharacterToSector(st_Character* character)
{
	int sec_y = character->sector.sec_y;
	int sec_x = character->sector.sec_x;

	g_sector[sec_y][sec_x].push_back(character);
}

void RemoveCharacterFromSector(st_Character* character)
{
	list<st_Character*>::iterator it;
	int sec_y = character->sector.sec_y;
	int sec_x = character->sector.sec_x;
	for (it = g_sector[sec_y][sec_x].begin(); it != g_sector[sec_y][sec_x].end(); ++it)
	{
		if ((*it) == character)
		{
			g_sector[sec_y][sec_x].remove(*it);
			break;
		}
	}
}


void AddUser(st_Character* character)
{
	g_character_map.insert({ character->character_id, character });
}

void	GetUpdateSectorAround
(st_Character* character, st_SECTOR_AROUND* remove_sector, st_SECTOR_AROUND* add_sector)
{
	int old_cnt, cur_cnt;
	bool find;
	st_SECTOR_AROUND old_sector_around, cur_sector_around;

	old_sector_around.count = 0;
	cur_sector_around.count = 0;
	remove_sector->count = 0;
	add_sector->count = 0;

	// cur old sector
	GetSectorAround(character->old_sector.sec_x, character->old_sector.sec_y, &old_sector_around);
	GetSectorAround(character->sector.sec_x, character->sector.sec_y, &cur_sector_around);

	for (old_cnt = 0; old_cnt < old_sector_around.count; ++old_cnt)
	{
		find = false;
		for (cur_cnt = 0; cur_cnt < cur_sector_around.count; ++cur_cnt)
		{
			if (old_sector_around.around[old_cnt].sec_x == cur_sector_around.around[cur_cnt].sec_x &&
				old_sector_around.around[old_cnt].sec_y == cur_sector_around.around[cur_cnt].sec_y)
			{
				find = true;
				break;
			}
		}
		if (find == false)
		{
			remove_sector->around[remove_sector->count] = old_sector_around.around[old_cnt];
			remove_sector->count++;
		}
	}

	for (cur_cnt = 0; cur_cnt < cur_sector_around.count; ++cur_cnt)
	{
		find = false;
		for (old_cnt = 0; old_cnt < old_sector_around.count; ++old_cnt)
		{
			if (old_sector_around.around[old_cnt].sec_x == cur_sector_around.around[cur_cnt].sec_x &&
				old_sector_around.around[old_cnt].sec_y == cur_sector_around.around[cur_cnt].sec_y)
			{
				find = true;
				break;
			}
		}
		if (find == false)
		{
			add_sector->around[add_sector->count] = cur_sector_around.around[cur_cnt];
			add_sector->count++;
		}
	}
}

void CharacterSectorUpdatePacket(st_Character* character)
{
	st_SECTOR_AROUND remove_sector, add_sector;
	st_Character* exist_character;

	list<st_Character*>* sector_list;
	list<st_Character*>::iterator list_iter;

	CSerialization buffer;
	int i;
	// Get remove add sector 
	GetUpdateSectorAround(character, &remove_sector, &add_sector);


	// my info -> remove sector
	for (i = 0; i < remove_sector.count; ++i)
	{
		NetworkUtil_DeleteSectorOneEraseMe(remove_sector.around[i].sec_x,
			remove_sector.around[i].sec_y, character->character_id);
	}

	// remove sector info -> me
	for (i = 0; i < remove_sector.count; ++i)
	{
		sector_list = &g_sector[remove_sector.around[i].sec_y][remove_sector.around[i].sec_x];

		for (list_iter = sector_list->begin(); list_iter != sector_list->end(); ++list_iter)
		{
			CreateDeleteCharacterPacket(buffer, (*list_iter)->session->session_id);
			SendPacket(character->session->session_id, buffer);
			buffer.Clear();
		}
	}

	// add sector - create Character and MoveStart
	// my info -> add sector
	for (i = 0; i < add_sector.count; ++i)
	{
		NetworkUtil_DeleteSectorOneEraseOthers(add_sector.around[i].sec_x, add_sector.around[i].sec_y, character);
	}


	// 4
	for (i = 0; i < add_sector.count; ++i)
	{
		sector_list = &g_sector[add_sector.around[i].sec_y][add_sector.around[i].sec_x];

		for (list_iter = sector_list->begin(); list_iter != sector_list->end(); ++list_iter)
		{
			exist_character = *list_iter;

			if (exist_character != character)
			{
				CSerialization buffer;
				CreateOtherCharacterPacket(buffer,
					exist_character->character_id,
					exist_character->direction,
					exist_character->x,
					exist_character->y,
					exist_character->hp);
				SendPacket(character->session->session_id, buffer);
				buffer.Clear();
				if (exist_character->action <= 7)
				{
					CreateMoveStartPacket(buffer,
						exist_character->character_id,
						exist_character->action,
						exist_character->x,
						exist_character->y);
					SendPacket(character->session->session_id, buffer);
					buffer.Clear();
				}
			}
		}
	}
}