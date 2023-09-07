#pragma comment(lib, "ws2_32")
#include <iostream>
#include <WinSock2.h>
#include <unordered_map>
#include <Windows.h>

#include "CSerializationBuffer.h"
#include "CMemoryPool.h"
#include "CRingBuffer.h"
#include "session.h"
#include "NetworkLibrary.h"
#include "CreateMessage.h"
#include "Util.h"

#include "PACKET_PROTOCOL.h"

using namespace std;

extern HANDLE g_job_event;
extern unordered_map<SESSEION_TYPE, st_session*> g_session_map;
extern CRITICAL_SECTION g_session_map_cs;
extern CRingBuffer g_job_queue;
extern CRITICAL_SECTION g_job_cs;
extern CMemoryPool<st_session> g_session_pool;

CMemoryPool<CSerialization> g_packet_pool(30000, true);

bool AcceptRecvPost(st_session& now_session)
{
	int WSARecv_ret;
	DWORD recvbytes;
	DWORD flags;

	// 다시 WSARecv를 걸어준다.
	WSABUF wsabuf;

	long IO_count_ret;

	// check ringbuffer Size
	wsabuf.buf = now_session.recv_buffer.GetBufferPtr();
	wsabuf.len = now_session.recv_buffer.DirectEnqueueSize();

	// Init Overlapped
	ZeroMemory(&now_session.recv_overlapped, sizeof(now_session.recv_overlapped));

	// Increment IO_Count
	IO_count_ret = InterlockedIncrement(&now_session.IO_count);

	// WSARecv
	flags = 0;
	WSARecv_ret = WSARecv(now_session.socket, &wsabuf, 1, &recvbytes, &flags, &now_session.recv_overlapped, NULL);
	if (WSARecv_ret == SOCKET_ERROR)
	{
		DWORD error_code = WSAGetLastError();
		if (error_code != ERROR_IO_PENDING) {
			IO_count_ret = InterlockedDecrement(&now_session.IO_count);
			if (IO_count_ret == 0)
			{
				printf("2. Accept 안 WSARecv()[%d] sp[%p] IO_COUNT[%d] \n", error_code, &now_session, IO_count_ret);
				ReleaseSession(now_session);
				return false;
			}
		}
	}
	return true;
}


void SendPost(st_session& now_session)
{
	// WSASend에 사용될 변수들
	int WSASend_ret;
	WSABUF wsabuf[2];
	int buffer_count = 1;
	DWORD sendbytes;
	DWORD flags;

	// send ring buffer에 사용될 변수
	int use_size;
	char* front_ptr;
	int dequeue_size;

	// Error check
	DWORD error_code;
	DWORD IO_count_ret;

	LONG prev_send_flag;

	prev_send_flag = InterlockedExchange(&now_session.send_flag, 1);
	if (prev_send_flag == 0)
	{
		use_size = now_session.send_buffer.GetUseSize();
		if (use_size > 0)
		{
			front_ptr = now_session.send_buffer.GetFrontBufferPtr();
			dequeue_size = now_session.send_buffer.DirectDequeueSize();

			// default buffer size is 1
			buffer_count = 1;

			// Calculate WSABUF
			wsabuf[0].buf = front_ptr;
			wsabuf[0].len = dequeue_size;
			if (use_size > dequeue_size)
			{
				wsabuf[1].buf = now_session.send_buffer.GetBufferPtr();
				wsabuf[1].len = use_size - dequeue_size;
				buffer_count++;
			}

			// Init Overlapped
			ZeroMemory(&now_session.send_overlapped, sizeof(now_session.send_overlapped));

			// Increment IO_Count
			IO_count_ret = InterlockedIncrement(&now_session.IO_count);

			// Send part
			flags = 0;
			WSASend_ret = WSASend(now_session.socket, wsabuf, buffer_count, &sendbytes, flags, &now_session.send_overlapped, NULL);
			if (WSASend_ret == SOCKET_ERROR)
			{
				error_code = WSAGetLastError();
				if (error_code != ERROR_IO_PENDING) {
					
					IO_count_ret = InterlockedDecrement(&now_session.IO_count);
					if (IO_count_ret == 0)
					{
						printf("2. WSASend()[%d] sid[%d] sp[%p] IO_COUNT[%d] \n", error_code, now_session.session_id, &now_session, IO_count_ret);
						DWORD val = now_session.session_id;
						bool flag = ReleaseSession(now_session);
						if (flag)
							OnRelease(val);
					}
				}
			}
		}
		else // 0이라면 빼줘야 함.
		{
			InterlockedExchange(&now_session.send_flag, 0);
		}
	}
}

void SendPacket(SESSEION_TYPE& session_id, CSerialization& packet)
{
	CSerialization total_packet;
	
	short len;
	short type;

	int enqueue_ret;
	__int64 data_size;

	// variables
	DWORD		c_id;	// 4
	DWORD		a_id;
	BYTE		dir;	// 1
	short		x;		
	short		y;
	char		hp;
	DWORD		time;

	// session
	SESSION_ITER_TYPE iter;
	st_session* now_session;

	// packet 넣기
	packet >> type;
	
	
	switch (type)
	{
	case dfPACKET_SC_CREATE_MY_CHARACTER: // 0
	{
		
		packet >> c_id >> dir >> x >> y >> hp;
		
		// new
		len = 12; // 2 | 2 | 10
		total_packet << len << type << c_id << dir << x << y << hp;
		break;
	}

	case dfPACKET_SC_CREATE_OTHER_CHARACTER: // 1
	{
		packet >> c_id >> dir >> x >> y >> hp;
		
		// new
		len = 12;  // 2 | 2 | 10
		total_packet << len << type << c_id << dir << x << y << hp;
		break;
	}
	case dfPACKET_SC_DELETE_CHARACTER: // 2
	{
		packet >> c_id;

		// new
		len = 6;
		total_packet << len << type << c_id;
		break;
	}
	case dfPACKET_SC_MOVE_START: // 11
	{
		packet >> c_id >> dir >> x >> y;

		len = 11;
		total_packet << len << type << c_id << dir << x << y;
		break;
	}
	case dfPACKET_SC_MOVE_STOP: // 13
	{
		packet >> c_id >> dir >> x >> y;
		len = 11;
		total_packet << len << type << c_id << dir << x << y;
		break;
	}
	{
	case dfPACKET_SC_ATTACK1:
	case dfPACKET_SC_ATTACK2:
	case dfPACKET_SC_ATTACK3:
		packet >> c_id >> dir >> x >> y;
		len = 11;
		total_packet << len << type << c_id << dir << x << y;
		break;
	}
	case dfPACKET_SC_DAMAGE:
	{
		packet >> a_id >> c_id >> hp;
		len = 11;
		total_packet << len << type << a_id << c_id << hp;
		break;
	}
	case dfPACKET_SC_SYNC: // 251
	{
		packet >> c_id >> x >> y;
		len = 10;
		total_packet << len << type << c_id << x << y;
		break;
	}
	case dfPACKET_SC_ECHO: // 253
	{
		packet >> time;
		len = 6;
		total_packet << len << type << time;
		break;
	}
	default:
		printf("SendPacket에서 session_id가 [%d]를 보냄\n", type);
		break;
	}

	// sendRingbuffer 사용
	EnterCriticalSection(&g_session_map_cs);

	iter = g_session_map.find(session_id);
	
	// 못찾았으니
	if (iter == g_session_map.end())
	{
		LeaveCriticalSection(&g_session_map_cs);
		return;
	}

	now_session = iter->second;

	EnterCriticalSection(&now_session->session_cs);
	data_size = total_packet.GetDataSize();
	enqueue_ret = now_session->send_buffer.Enqueue(total_packet.GetBufferPtr(), (int)data_size);
	LeaveCriticalSection(&now_session->session_cs);

	if (enqueue_ret != data_size)
	{
		printf("send Enqueue결함[%d] sessionid[%d] cid[%d] type[%d] [%p]\n", enqueue_ret, session_id, c_id, type, &now_session);
		Disconnect(session_id);
		LeaveCriticalSection(&g_session_map_cs);
		return;
	}

	SendPost(*now_session);
	LeaveCriticalSection(&g_session_map_cs);
}


void OnMessage(SESSEION_TYPE& session_id, CSerialization& packet)
{
	CSerialization* job_packet;


	// message 변수들
	unsigned short type;
	BYTE dir;
	short x, y;
	DWORD time;

	// 리턴 변수
	int enqueue_ret;

	
	job_packet = g_packet_pool.Alloc();
	job_packet->Clear();
	packet >> type;
	switch (type)
	{
	case dfPACKET_CS_MOVE_START:
	{
		packet >> dir >> x >> y;
		CreateMoveStartJobPacket(*job_packet, session_id, dir, x, y);
		break;
	}
	case dfPACKET_CS_MOVE_STOP:
	{
		packet >> dir >> x >> y;
		CreateMoveStopJobPacket(*job_packet, session_id, dir, x, y);
		break;
	}
	case dfPACKET_CS_ATTACK1:
	{
		packet >> dir >> x >> y;
		CreateAttack1JobPacket(*job_packet, session_id, dir, x, y);
		break;
	}
	case dfPACKET_CS_ATTACK2:
	{
		packet >> dir >> x >> y;
		CreateAttack2JobPacket(*job_packet, session_id, dir, x, y);
		break;
	}
	case dfPACKET_CS_ATTACK3:
	{
		packet >> dir >> x >> y;
		CreateAttack3JobPacket(*job_packet, session_id, dir, x, y);
		break;
	}
	case dfPACKET_CS_ECHO:
	{
		packet >> time;
		CreateEchoJobPacket(*job_packet, session_id, time);
		break;
	}
	default:
	{
		printf("OnMessage Error type find[%d]\n", type);
		return ;
	}
	}

	EnterCriticalSection(&g_job_cs);
	enqueue_ret = g_job_queue.Enqueue((char *)&job_packet, 8);
	if (enqueue_ret != 8)
	{
		printf("JobQueue Enqueue 실패 %d가 들어감\n", enqueue_ret);
		// 1. 터트리자
		// 2. 큐 안의 개수를 보고 속도를 조절한다.
		// 보류.
	}
	LeaveCriticalSection(&g_job_cs);
	SetEvent(g_job_event);
}

st_Character* InitCharacter(st_session* now_session)
{
	st_Character* new_client;

	new_client = new st_Character;

	new_client->session = now_session;
	new_client->character_id = now_session->session_id;

	return new_client;
}

void OnClientJoin(SESSEION_TYPE& session_id, st_session &now_session)
{
	CSerialization* job_packet;
	int enqueue_ret;
	short type;
	st_Character* new_client;

	new_client = InitCharacter(&now_session);

	job_packet = g_packet_pool.Alloc();
	job_packet->Clear();

	type = dfPACKET_CS_CLIENT_JOIN;

	// data 삽입

	uintptr_t ptr = (uintptr_t) new_client;
	*job_packet << session_id << type << ptr;

	EnterCriticalSection(&g_job_cs);
	enqueue_ret = g_job_queue.Enqueue((char*)&job_packet, 8);
	if (enqueue_ret != 8)
	{
		printf("OnClientJoin 8byte가 아닌 %d가 들어감\n", enqueue_ret);
		// 1. 터트리자
		// 2. 큐 안의 개수를 보고 속도를 조절한다.
		// 보류.
	}
	LeaveCriticalSection(&g_job_cs);
	SetEvent(g_job_event);
}

void OnRelease(SESSEION_TYPE& session_id)
{
	CSerialization* job_packet;
	int enqueue_ret;
	short type;

	job_packet = g_packet_pool.Alloc();
	job_packet->Clear();

	type = dfPACKET_CS_CLIENT_LEAVE;

	*job_packet << session_id << type;

	EnterCriticalSection(&g_job_cs);
	enqueue_ret = g_job_queue.Enqueue((char*)&job_packet, 8);
	if (enqueue_ret != 8)
	{
		printf("OnRelease 8byte가 아닌 %d가 들어감\n", enqueue_ret);
		// 1. 터트리자
		// 2. 큐 안의 개수를 보고 속도를 조절한다.
		// 보류.
	}
	LeaveCriticalSection(&g_job_cs);
	SetEvent(g_job_event);
}

void RecvPost(st_session& now_session)
{
	int WSARecv_ret;
	DWORD recvbytes;
	DWORD flags;

	long IO_count_ret;

	// 다시 WSARecv를 걸어준다.
	WSABUF wsabuf[2];

	// check ringbuffer Size
	int free_size = now_session.recv_buffer.GetFreeSize();
	int enqueue_size = now_session.recv_buffer.DirectEnqueueSize();
	int buffer_count = 1;

	wsabuf[0].buf = now_session.recv_buffer.GetRearBufferPtr();
	wsabuf[0].len = enqueue_size;
	if (free_size > enqueue_size)
	{
		wsabuf[1].buf = now_session.recv_buffer.GetBufferPtr();
		wsabuf[1].len = free_size - enqueue_size;
		buffer_count++;
	}

	// Init Overlapped
	ZeroMemory(&now_session.recv_overlapped, sizeof(now_session.recv_overlapped));

	// Increment IO_Count
	IO_count_ret = InterlockedIncrement(&now_session.IO_count);


	// WSARecv
	flags = 0;
	WSARecv_ret = WSARecv(now_session.socket, wsabuf, buffer_count, &recvbytes, &flags, &now_session.recv_overlapped, NULL);
	if (WSARecv_ret == SOCKET_ERROR)
	{
		DWORD error_code = WSAGetLastError();
		if (error_code != ERROR_IO_PENDING) {
			IO_count_ret = InterlockedDecrement(&now_session.IO_count);
			if (IO_count_ret == 0)
			{
				DWORD val = now_session.session_id;
				printf("2. WSARecv()[%d] sid[%d] sp[%p] IO_COUNT[%d] \n", error_code, val, &now_session, IO_count_ret);
				bool flag = ReleaseSession(now_session);
				if (flag)
					OnRelease(val);
			}
		}
	}
}

bool ReleaseSession(st_session& now_session)
{
	bool release_success_flag = true;
	SOCKET session_sock;
	unordered_map<SESSEION_TYPE, st_session*>::iterator iter;

	EnterCriticalSection(&g_session_map_cs);
	iter = g_session_map.find(now_session.session_id);
	if (iter == g_session_map.end())
	{
		printf("이미 지워진 세션 ID[%lu]\n", now_session.session_id);
		release_success_flag = false;
	}
	else
	{
		session_sock = iter->second->socket;
		iter->second->socket = INVALID_SOCKET;
		g_session_map.erase(iter);
		if (session_sock != INVALID_SOCKET)
			closesocket(session_sock);
	}
	LeaveCriticalSection(&g_session_map_cs);
	if (release_success_flag)
		g_session_pool.Free(&now_session);

	return release_success_flag;
}

bool Disconnect(SESSEION_TYPE& session_id)
{
	SOCKET session_sock;
	unordered_map<SESSEION_TYPE, st_session*>::iterator iter;
	st_session* session;


	EnterCriticalSection(&g_session_map_cs);
	iter = g_session_map.find(session_id);
	if (iter != g_session_map.end())
	{	
		session = iter->second;
		session_sock = session->socket;
		session->socket = INVALID_SOCKET;
		if (session_sock != INVALID_SOCKET)
			closesocket(session_sock);
		LeaveCriticalSection(&g_session_map_cs);
		return true;
	}
	else
	{
		printf("Disconnect 안에서 세션 맵에서[%d]가 존재하지 않습니다\n", session_id);
	}
	LeaveCriticalSection(&g_session_map_cs);
	return false;
}