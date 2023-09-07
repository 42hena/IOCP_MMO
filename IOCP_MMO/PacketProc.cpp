#include <iostream>
#include <process.h>
#include <unordered_map>
#include <Windows.h>

#include "CMemoryPool.h"
#include "CSerializationBuffer.h"
#include "CRingBuffer.h"
#include "session.h"
#include "NetworkLibrary.h"
#include "PACKET_PROTOCOL.h"
#include "Util.h"
#include "CreateMessage.h"

using namespace std;

extern list< st_Character* > g_sector[dfSECTOR_MAX_Y + 1][dfSECTOR_MAX_X + 1];
extern unordered_map<DWORD, st_Character*> g_character_map;



bool	PacketProc_ClientJoin(SESSEION_TYPE& session_id, CSerialization& buffer)
{
	st_Character* new_client;
	st_Character* character;
	CSerialization packet;
	uintptr_t ptr;
	st_SECTOR_AROUND around;
	int sec_x;
	int sec_y;
	int i;

	//
	DWORD s_id;
	BYTE dir;
	short x;
	short y;
	BYTE hp;

	list<st_Character*>::iterator iter;


	buffer >> ptr;
	new_client = (st_Character*)ptr;
	AddUser(new_client);
	AddCharacterToSector(new_client);

	// 1. ������ ���� �ɸ��� ����
	CreateMyCharacterPacket(packet, session_id, new_client->direction, new_client->x, new_client->y, new_client->hp);
	SendPacket(session_id, packet);


	GetSectorAround(new_client->sector.sec_x, new_client->sector.sec_y, &around);

	// 2. �� ��ǥ�� �ٸ� ������� �ִ� ����.
	for (i = 0; i < around.count; ++i)
	{
		sec_x = around.around[i].sec_x;
		sec_y = around.around[i].sec_y;
		SectorOne_CreateOthers(sec_x, sec_y, new_client);

		// 3. �ٸ� ��� ��ǥ�� ������
		// cur_sector_character_list = g_sector[sec_y][sec_x]; // ������ �� �� ����.

		for (iter = g_sector[sec_y][sec_x].begin(); iter != g_sector[sec_y][sec_x].end(); ++iter)
		{
			character = *iter;
			if (character != new_client)
			{
				s_id = character->session->session_id;
				dir = character->direction;
				x = character->x;
				y = character->y;
				hp = character->hp;
				CreateOtherCharacterPacket(buffer, s_id, dir, x, y, hp);
				SendPacket(session_id, buffer);
				buffer.Clear();
				if (character->action != 8)
				{
					CreateMoveStartPacket(buffer, s_id, dir, x, y);
					SendPacket(session_id, buffer);
					buffer.Clear();
				}
			}
		}
	}

	return true;
}

void	PacketProc_ClientLeave(SESSEION_TYPE& session_id)
{
	st_Character* character;
	unordered_map<DWORD, st_Character*>::iterator iter;
	st_SECTOR_AROUND around;
	int i;

	iter = g_character_map.find(session_id);
	if (iter != g_character_map.end())
	{
		character = iter->second;
		g_character_map.erase(iter);

		GetSectorAround(character->sector.sec_x, character->sector.sec_y, &around);
		// RemoveCharacterFromSector(character);
		g_sector[character->sector.sec_y][character->sector.sec_x].remove(character); // TODO: �ٽ�
		for (i = 0; i < around.count; ++i)
		{
			SectorOne_DeleteOthers(around.around[i].sec_x, around.around[i].sec_y, character);
		}
		delete character;
	}
	else
	{
		printf("ClientLeave In �������?[%d]\n", session_id);
	}
	
}


// Sync ó�� ����
void	PacketProc_MoveStartPacket(SESSEION_TYPE& session_id, CSerialization& buffer)
{
	// �޽��� ������
	BYTE dir;
	short x;
	short y;

	// map ���� ������
	unordered_map<DWORD, st_Character*>::iterator iter;
	st_Character* character;

	// sector ����
	st_SECTOR_AROUND cur;

	int i;

/*
* code ����
*/


	iter = g_character_map.find(session_id);
	if (iter == g_character_map.end())
	{
		printf("Error PacketProc_MoveStartPacket\n");
		return;
	}
	buffer >> dir >> x >> y;
	character = iter->second;
	
	// Sync ó�� ��Ʈ 
	if (abs(character->x - x) > dfERROR_RANGE || abs(character->y - y) > dfERROR_RANGE)
	{
		GetSectorAround(character->x / dfSECTOR_SIZE, character->y / dfSECTOR_SIZE, &cur);
		for (i = 0; i < cur.count; ++i)
		{
			SectorOne_SyncOthers(cur.around[i].sec_x, cur.around[i].sec_y, character);
		}
		y = character->y;
		x = character->x;
	}

	// decide move direction
	character->action = dir;

	switch (dir) // TODO: assembly �����ϴ� ����.
	{
	case dfPACKET_MOVE_DIR_LL:
	case dfPACKET_MOVE_DIR_LU:
	case dfPACKET_MOVE_DIR_LD:
		character->direction = dfPACKET_MOVE_DIR_LL;
		break;
	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RD:
		character->direction = dfPACKET_MOVE_DIR_RR;
		break;
	}

	// cur pos [y, x] update
	character->y = y;
	character->x = x;

	// Sector ������Ʈ
	if (SectorProc_UpdateCharacter(character))
	{
		CharacterSectorUpdatePacket(character);
	}

	// old Sector �ٲٱ�.
	character->old_sector.sec_y = character->sector.sec_y;
	character->old_sector.sec_x = character->sector.sec_x;

	// ��ó sector ã��
	GetSectorAround(character->sector.sec_x, character->sector.sec_y, &cur);

	// ���� ������ sector���� �޽��� ������
	for (i = 0; i < cur.count; ++i)
	{
		SectorOne_MoveStartOthers(cur.around[i].sec_x, cur.around[i].sec_y, character);
	}
}
// Sync ó�� ����
void	PacketProc_MoveStopPacket(SESSEION_TYPE& session_id, CSerialization& buffer)
{
	// �޽��� ������
	BYTE dir;
	short x;
	short y;

	// map
	unordered_map<DWORD, st_Character*>::iterator iter;
	st_Character* character;

	// sector
	st_SECTOR_AROUND cur;

	int i;

/*
* code ����
*/

	iter = g_character_map.find(session_id);
	if (iter == g_character_map.end())
	{
		printf("[PacketProc_MoveStopPacket] Error\n");
		return;
	}
	buffer >> dir >> x >> y;
	character = iter->second;


	// SYNC : TODO
	if (abs(character->x - x) > dfERROR_RANGE || abs(character->y - y) > dfERROR_RANGE)
	{
		GetSectorAround(character->x / dfSECTOR_SIZE, character->y / dfSECTOR_SIZE, &cur);
		for (i = 0; i < cur.count; ++i)
		{
			SectorOne_SyncOthers(cur.around[i].sec_x, cur.around[i].sec_y, character);
		}
		y = character->y;
		x = character->x;
	}

	character->action = 8;
	switch (dir)
	{
	case dfPACKET_MOVE_DIR_LL:
	case dfPACKET_MOVE_DIR_LU:
	case dfPACKET_MOVE_DIR_LD:
		character->direction = dfPACKET_MOVE_DIR_LL;
		break;
	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RD:
		character->direction = dfPACKET_MOVE_DIR_RR;
		break;
	}

	// cur pos [y, x] update
	character->y = y;
	character->x = x;

	// sector ������Ʈ
	if (SectorProc_UpdateCharacter(character))
	{
		CharacterSectorUpdatePacket(character);
		
		// old Sector �ٲٱ�.
		character->old_sector.sec_y = character->sector.sec_y;
		character->old_sector.sec_x = character->sector.sec_x;
	}


	// ���� �� ���
	GetSectorAround(character->sector.sec_x, character->sector.sec_y, &cur);

	// ���� ������ sector���� �޽��� ������
	for (i = 0; i < cur.count; ++i)
	{
		SectorOne_MoveStopOthers(cur.around[i].sec_x, cur.around[i].sec_y, character);
	}
}

void	PacketProc_Attack1Packet(SESSEION_TYPE& session_id, CSerialization& buffer)
{
	// �޽��� ������
	BYTE dir;
	short x;
	short y;

	// map
	unordered_map<DWORD, st_Character*>::iterator iter;

	// character
	st_Character* character;
	st_Character* victim;


	// sector
	st_SECTOR_AROUND cur;
	st_SECTOR_AROUND around;


	short now_x;
	short now_y;

	//list<st_Character*> sector;
	list<st_Character*>::iterator list_iter;

	int i, j;

	
/*
* code ����
*/

	iter = g_character_map.find(session_id);
	if (iter == g_character_map.end())
	{
		printf("[PacketProc_Attack1Packet] Error\n");
		return;
	}
	buffer >> dir >> x >> y;
	character = iter->second;

	// Sync ó�� ��Ʈ 
	if (abs(character->x - x) > dfERROR_RANGE || abs(character->y - y) > dfERROR_RANGE)
	{
		GetSectorAround(character->x / dfSECTOR_SIZE, character->y / dfSECTOR_SIZE, &cur);
		for (i = 0; i < cur.count; ++i)
		{
			SectorOne_SyncOthers(cur.around[i].sec_x, cur.around[i].sec_y, character);
		}
		y = character->y;
		x = character->x;
	}



	// cur pos [y, x] update
	character->y = y;
	character->x = x;

	// sector ������Ʈ
	if (SectorProc_UpdateCharacter(character))
	{
		CharacterSectorUpdatePacket(character);

		// old Sector �ٲٱ�.
		character->old_sector.sec_y = character->sector.sec_y;
		character->old_sector.sec_x = character->sector.sec_x;
	}


	now_x = character->x;
	now_y = character->y;

	// ���� �ֺ� ���ϱ�
	
	GetSectorAround(character->sector.sec_x, character->sector.sec_y, &cur);

	// ���� ������ ���� 1�� ���ο��� ����
	for (i = 0; i < cur.count; ++i)
	{
		SectorOne_Attack1(cur.around[i].sec_x,
			cur.around[i].sec_y, character);
	}

	// ������ �� �� ã��
	if (dir == dfPACKET_MOVE_DIR_LL)
	{
		for (i = 0; i < cur.count; ++i)
		{
			// sector = g_sector[cur.around[i].sec_y][cur.around[i].sec_x];
			GetSectorAround(cur.around[i].sec_x, cur.around[i].sec_y, &around);

			for (list_iter = g_sector[cur.around[i].sec_y][cur.around[i].sec_x].begin(); list_iter != g_sector[cur.around[i].sec_y][cur.around[i].sec_x].end(); ++list_iter)
			{
				victim = *list_iter;
				if ((now_y - dfATTACK1_RANGE_Y) <= victim->y && victim->y <= now_y + dfATTACK1_RANGE_Y &&
					(now_x - dfATTACK1_RANGE_X) <= victim->x && victim->x <= now_x - 1)
				{
					if (victim->hp > 0)
					{
						victim->hp -= dfATTACK1_DAMAGE;
						for (j = 0; j < around.count; ++j)
						{
							SectorOne_Damage(around.around[j].sec_x, around.around[j].sec_y, NULL, victim->character_id, victim->hp, session_id);
						}
					}
				}
			}
		}
	}
	else if (dir == dfPACKET_MOVE_DIR_RR)
	{
		for (i = 0; i < cur.count; ++i)
		{
			//sector = g_sector[cur.around[i].sec_y][cur.around[i].sec_x];
			GetSectorAround(cur.around[i].sec_x, cur.around[i].sec_y, &around);

			for (list_iter = g_sector[cur.around[i].sec_y][cur.around[i].sec_x].begin(); list_iter != g_sector[cur.around[i].sec_y][cur.around[i].sec_x].end(); ++list_iter)
			{
				victim = *list_iter;
				if ((now_y - dfATTACK1_RANGE_Y) <= victim->y && victim->y <= now_y + dfATTACK1_RANGE_Y &&
					(now_x + 1 <= victim->x && victim->x <= now_x + dfATTACK1_RANGE_X))
				{
					if (victim->hp > 0)
					{
						victim->hp -= dfATTACK1_DAMAGE;
						for (j = 0; j < around.count; ++j)
						{
							SectorOne_Damage(around.around[j].sec_x, around.around[j].sec_y, NULL, victim->character_id, victim->hp, session_id);
						}
					}
				}
			}
		}
	}
	else
	{
		printf("Error ���� ���� �ȵ�\n");
	}
}

//
void	PacketProc_Attack2Packet(SESSEION_TYPE& session_id, CSerialization& buffer)
{
	// �޽��� ������
	BYTE dir;
	short x;
	short y;

	// map
	unordered_map<DWORD, st_Character*>::iterator iter;

	// character
	st_Character* character;
	st_Character* victim;


	// sector
	st_SECTOR_AROUND cur;
	st_SECTOR_AROUND around;


	short now_x;
	short now_y;

	list<st_Character*> sector;
	list<st_Character*>::iterator list_iter;

	int i, j;

/*
* code ����
*/

	iter = g_character_map.find(session_id);
	if (iter == g_character_map.end())
	{
		printf("[PacketProc_Attack2Packet] Error\n");
		return;
	}
	buffer >> dir >> x >> y;
	character = iter->second;

	// Sync ó�� ��Ʈ 
	if (abs(character->x - x) > dfERROR_RANGE || abs(character->y - y) > dfERROR_RANGE)
	{
		GetSectorAround(character->x / dfSECTOR_SIZE, character->y / dfSECTOR_SIZE, &cur);
		for (i = 0; i < cur.count; ++i)
		{
			SectorOne_SyncOthers(cur.around[i].sec_x, cur.around[i].sec_y, character);
		}
		y = character->y;
		x = character->x;
	}



	// cur pos [y, x] update
	character->y = y;
	character->x = x;

	// sector ������Ʈ
	if (SectorProc_UpdateCharacter(character))
	{
		CharacterSectorUpdatePacket(character);

		// old Sector �ٲٱ�.
		character->old_sector.sec_y = character->sector.sec_y;
		character->old_sector.sec_x = character->sector.sec_x;
	}

	now_x = character->x;
	now_y = character->y;

	// ���� �ֺ� ���ϱ�

	GetSectorAround(character->sector.sec_x, character->sector.sec_y, &cur);

	// ���� ������ ���� 2�� ���ο��� ����
	for (i = 0; i < cur.count; ++i)
	{
		SectorOne_Attack2(cur.around[i].sec_x,
			cur.around[i].sec_y, character);
	}

	// ������ �� �� ã��
	if (dir == dfPACKET_MOVE_DIR_LL)
	{
		for (i = 0; i < cur.count; ++i)
		{
			//sector = g_sector[cur.around[i].sec_y][cur.around[i].sec_x];
			GetSectorAround(cur.around[i].sec_x, cur.around[i].sec_y, &around);

			for (list_iter = g_sector[cur.around[i].sec_y][cur.around[i].sec_x].begin(); list_iter != g_sector[cur.around[i].sec_y][cur.around[i].sec_x].end(); ++list_iter)
			{
				victim = *list_iter;
				if ((now_y - dfATTACK2_RANGE_Y) <= victim->y && victim->y <= now_y + dfATTACK2_RANGE_Y &&
					(now_x - dfATTACK2_RANGE_X) <= victim->x && victim->x <= now_x - 1)
				{
					if (victim->hp > 0)
					{
						victim->hp -= dfATTACK2_DAMAGE;
						for (j = 0; j < around.count; ++j)
						{
							SectorOne_Damage(around.around[j].sec_x, around.around[j].sec_y, NULL, victim->character_id, victim->hp, session_id);
						}
					}
				}
			}
		}
	}
	else if (dir == dfPACKET_MOVE_DIR_RR)
	{
		for (i = 0; i < cur.count; ++i)
		{
			//sector = g_sector[cur.around[i].sec_y][cur.around[i].sec_x];
			GetSectorAround(cur.around[i].sec_x, cur.around[i].sec_y, &around);

			for (list_iter = g_sector[cur.around[i].sec_y][cur.around[i].sec_x].begin(); list_iter != g_sector[cur.around[i].sec_y][cur.around[i].sec_x].end(); ++list_iter)
			{
				victim = *list_iter;
				if ((now_y - dfATTACK2_RANGE_Y) <= victim->y && victim->y <= now_y + dfATTACK2_RANGE_Y &&
					(now_x + 1 <= victim->x && victim->x <= now_x + dfATTACK2_RANGE_X))
				{
					if (victim->hp > 0)
					{
						victim->hp -= dfATTACK2_DAMAGE;
						for (j = 0; j < around.count; ++j)
						{
							SectorOne_Damage(around.around[j].sec_x, around.around[j].sec_y, NULL, victim->character_id, victim->hp, session_id);
						}
					}
				}
			}
		}
	}
	else
	{
		printf("Error ���� ���� �ȵ�\n");
	}
}

void	PacketProc_Attack3Packet(SESSEION_TYPE& session_id, CSerialization& buffer)
{
	// �޽��� ������
	BYTE dir;
	short x;
	short y;

	// map
	unordered_map<DWORD, st_Character*>::iterator iter;

	// character
	st_Character* character;
	st_Character* victim;


	// sector
	st_SECTOR_AROUND cur;
	st_SECTOR_AROUND around;


	short now_x;
	short now_y;

	//list<st_Character*> sector;
	list<st_Character*>::iterator list_iter;

	int i, j;


/*
* code ����
*/

	iter = g_character_map.find(session_id);
	if (iter == g_character_map.end())
	{
		printf("[PacketProc_Attack3Packet] Error\n");
		return;
	}
	buffer >> dir >> x >> y;
	character = iter->second;

	// Sync ó�� ��Ʈ 
	if (abs(character->x - x) > dfERROR_RANGE || abs(character->y - y) > dfERROR_RANGE)
	{
		GetSectorAround(character->x / dfSECTOR_SIZE, character->y / dfSECTOR_SIZE, &cur);
		for (i = 0; i < cur.count; ++i)
		{
			SectorOne_SyncOthers(cur.around[i].sec_x, cur.around[i].sec_y, character);
		}
		y = character->y;
		x = character->x;
	}

	// cur pos [y, x] update
	character->y = y;
	character->x = x;

	// sector ������Ʈ
	if (SectorProc_UpdateCharacter(character))
	{
		CharacterSectorUpdatePacket(character);

		// old Sector �ٲٱ�.
		character->old_sector.sec_y = character->sector.sec_y;
		character->old_sector.sec_x = character->sector.sec_x;
	}

	now_x = character->x;
	now_y = character->y;

	// ���� �ֺ� ���ϱ�
	GetSectorAround(character->sector.sec_x, character->sector.sec_y, &cur);

	// ���� ������ ���� 3�� ���ο��� ����
	for (i = 0; i < cur.count; ++i)
	{
		SectorOne_Attack3(cur.around[i].sec_x,
			cur.around[i].sec_y, character);
	}

	// ������ �� �� ã��
	if (dir == dfPACKET_MOVE_DIR_LL)
	{
		for (i = 0; i < cur.count; ++i)
		{
			//sector = g_sector[cur.around[i].sec_y][cur.around[i].sec_x];
			GetSectorAround(cur.around[i].sec_x, cur.around[i].sec_y, &around);

			for (list_iter = g_sector[cur.around[i].sec_y][cur.around[i].sec_x].begin(); list_iter != g_sector[cur.around[i].sec_y][cur.around[i].sec_x].end(); ++list_iter)
			{
				victim = *list_iter;
				if ((now_y - dfATTACK3_RANGE_Y) <= victim->y && victim->y <= now_y + dfATTACK3_RANGE_Y &&
					(now_x - dfATTACK3_RANGE_X) <= victim->x && victim->x <= now_x - 1)
				{
					if (victim->hp > 0)
					{
						victim->hp -= dfATTACK3_DAMAGE;
						for (j = 0; j < around.count; ++j)
						{
							SectorOne_Damage(around.around[j].sec_x, around.around[j].sec_y, NULL, victim->character_id, victim->hp, session_id);
						}
					}
				}
			}
		}
	}
	else if (dir == dfPACKET_MOVE_DIR_RR)
	{
		for (i = 0; i < cur.count; ++i)
		{
			//sector = g_sector[cur.around[i].sec_y][cur.around[i].sec_x];
			GetSectorAround(cur.around[i].sec_x, cur.around[i].sec_y, &around);

			for (list_iter = g_sector[cur.around[i].sec_y][cur.around[i].sec_x].begin(); list_iter != g_sector[cur.around[i].sec_y][cur.around[i].sec_x].end(); ++list_iter)
			{
				victim = *list_iter;
				if ((now_y - dfATTACK3_RANGE_Y) <= victim->y && victim->y <= now_y + dfATTACK3_RANGE_Y &&
					(now_x + 1 <= victim->x && victim->x <= now_x + dfATTACK3_RANGE_X))
				{
					if (victim->hp > 0)
					{
						victim->hp -= dfATTACK3_DAMAGE;
						for (j = 0; j < around.count; ++j)
						{
							SectorOne_Damage(around.around[j].sec_x, around.around[j].sec_y, NULL, victim->character_id, victim->hp, session_id);
						}
					}
				}
			}
		}
	}
	else
	{
		printf("Error ���� ���� �ȵ�\n");
	}
}

void	PacketProc_Echo(SESSEION_TYPE& session_id, CSerialization& buffer)
{
	// �޽��� ������
	DWORD time;

	CSerialization packet;

	/*
	* code ����
	*/

	buffer >> time;

	CreateEchoPacket(packet, time);
	SendPacket(session_id, packet);
}


bool CheckCharacterMove(short y, short x)
{
	if (y < 0 || y > dfRANGE_MOVE_BOTTOM || x < 0 || x > dfRANGE_MOVE_RIGHT)
		return false;
	else
		return true;
}

DWORD g_now_time;
DWORD g_next_time;
DWORD g_roop_count;
DWORD total_time;

void Update()
{
	unordered_map<DWORD, st_Character*>::iterator iter;
	// �ð� ��� return
	g_next_time = timeGetTime();
	total_time += g_next_time - g_now_time;
	g_now_time = g_next_time;
	while (total_time >= 40)
	{
		g_roop_count++;
		total_time -= 40;
		st_Character* character;

		for (iter = g_character_map.begin(); iter != g_character_map.end(); ++iter)
		{
			character = iter->second;
			if (character->hp <= 0)
			{
				Disconnect(character->session->session_id);
				continue;
			}
			else
			{
				if (g_now_time > character->session->last_recv_time + dfNETWORK_PACKET_RECV_TIMEOUT)
				{
					Disconnect(character->session->session_id);
					continue;
				}
				switch (character->action)
				{
				case dfPACKET_MOVE_DIR_LL:
				{
					if (CheckCharacterMove(character->y, character->x - dfSPEED_PLAYER_X))
					{
						character->x -= dfSPEED_PLAYER_X;
						if (SectorProc_UpdateCharacter(character))
						{
							CharacterSectorUpdatePacket(character);
							character->old_sector.sec_x = character->sector.sec_x;
							character->old_sector.sec_y = character->sector.sec_y;
						}
					}
					break;
				}
				case dfPACKET_MOVE_DIR_LU:
				{
					if (CheckCharacterMove(character->y - dfSPEED_PLAYER_Y, character->x - dfSPEED_PLAYER_X))
					{
						character->y -= dfSPEED_PLAYER_Y;
						character->x -= dfSPEED_PLAYER_X;
						if (SectorProc_UpdateCharacter(character))
						{
							CharacterSectorUpdatePacket(character);
							character->old_sector.sec_x = character->sector.sec_x;
							character->old_sector.sec_y = character->sector.sec_y;
						}
					}
					break;
				}
				case dfPACKET_MOVE_DIR_UU:
				{
					if (CheckCharacterMove(character->y - dfSPEED_PLAYER_Y, character->x))
					{
						character->y -= dfSPEED_PLAYER_Y;
						if (SectorProc_UpdateCharacter(character))
						{
							CharacterSectorUpdatePacket(character);
							character->old_sector.sec_x = character->sector.sec_x;
							character->old_sector.sec_y = character->sector.sec_y;
						}
					}
					break;
				}
				case dfPACKET_MOVE_DIR_RU:
				{
					if (CheckCharacterMove(character->y - dfSPEED_PLAYER_Y, character->x + dfSPEED_PLAYER_X))
					{
						character->y -= dfSPEED_PLAYER_Y;
						character->x += dfSPEED_PLAYER_X;
						if (SectorProc_UpdateCharacter(character))
						{
							CharacterSectorUpdatePacket(character);
							character->old_sector.sec_x = character->sector.sec_x;
							character->old_sector.sec_y = character->sector.sec_y;
						}
					}
					break;
				}

				case dfPACKET_MOVE_DIR_RR:
				{
					if (CheckCharacterMove(character->y, character->x + dfSPEED_PLAYER_X))
					{
						character->x += dfSPEED_PLAYER_X;
						if (SectorProc_UpdateCharacter(character))
						{
							CharacterSectorUpdatePacket(character);
							character->old_sector.sec_x = character->sector.sec_x;
							character->old_sector.sec_y = character->sector.sec_y;
						}
					}
					break;
				}

				case dfPACKET_MOVE_DIR_RD:
				{
					if (CheckCharacterMove(character->y + dfSPEED_PLAYER_Y, character->x + dfSPEED_PLAYER_X))
					{
						character->y += dfSPEED_PLAYER_Y;
						character->x += dfSPEED_PLAYER_X;
						if (SectorProc_UpdateCharacter(character))
						{
							CharacterSectorUpdatePacket(character);
							character->old_sector.sec_x = character->sector.sec_x;
							character->old_sector.sec_y = character->sector.sec_y;
						}
					}
					break;
				}

				case dfPACKET_MOVE_DIR_DD:
				{
					if (CheckCharacterMove(character->y + dfSPEED_PLAYER_Y, character->x))
					{
						character->y += dfSPEED_PLAYER_Y;
						if (SectorProc_UpdateCharacter(character))
						{
							CharacterSectorUpdatePacket(character);
							character->old_sector.sec_x = character->sector.sec_x;
							character->old_sector.sec_y = character->sector.sec_y;
						}
					}
					break;
				}

				case dfPACKET_MOVE_DIR_LD:
				{
					if (CheckCharacterMove(character->y + dfSPEED_PLAYER_Y, character->x - dfSPEED_PLAYER_X))
					{
						character->y += dfSPEED_PLAYER_Y;
						character->x -= dfSPEED_PLAYER_X;
						if (SectorProc_UpdateCharacter(character))
						{
							CharacterSectorUpdatePacket(character);
							character->old_sector.sec_x = character->sector.sec_x;
							character->old_sector.sec_y = character->sector.sec_y;
						}
					}
					break;
				}
				}// SWITCH��
				
			}
		}
	}
}
