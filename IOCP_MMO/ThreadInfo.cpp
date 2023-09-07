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
#include "CreateMessage.h"
#include "PacketProc.h"

using namespace std;

// global session_id
DWORD g_session_id;

// global session_map
CRITICAL_SECTION g_session_map_cs;
unordered_map<SESSEION_TYPE, st_session*> g_session_map;

// global exit flag
bool g_exit_flag;

// global jobQ event
HANDLE g_job_event;
CRingBuffer g_job_queue(600000);
CRITICAL_SECTION g_job_cs;
CMemoryPool<st_session> g_session_pool(7000, false);

extern SOCKET g_listen_sock;

unsigned int WINAPI AcceptThread(LPVOID arg)
{
	// 데이터 통신에 사용할 변수
	SOCKET client_sock;
	SOCKADDR_IN client_address;
	int address_length;
	HANDLE hcp = (HANDLE)arg;
	bool AcceptRecvPost_ret;
	unordered_map<SESSEION_TYPE, st_session*>::iterator iter;

	address_length = sizeof(client_address);

	while (true)
	{
		// accept()
		client_sock = accept(g_listen_sock, (SOCKADDR*)&client_address, &address_length);
		if (client_sock == INVALID_SOCKET)
		{
			printf("accept() Error[%d]\n", GetLastError());
			continue;
		}

		SESSEION_TYPE my_session_id;
		
		my_session_id = InterlockedIncrement(&g_session_id);
		
		// new_session 초기화
		st_session* new_session;
		new_session = g_session_pool.Alloc();
		new_session->socket = client_sock;
		new_session->IO_count = 0;
		new_session->session_id = my_session_id;
		new_session->send_flag = 0;
		new_session->send_buffer.ClearBuffer();
		new_session->recv_buffer.ClearBuffer();

		// session_map에 추가
		EnterCriticalSection(&g_session_map_cs);
		SESSION_ITER_TYPE iter = g_session_map.find(my_session_id);
		if (iter == g_session_map.end())
			g_session_map.insert({ my_session_id , new_session });
		else
		{
			printf("AcceptThread에서 session [%d]가 존재함.\n", my_session_id);
			// closesocket(client_sock); 일단 지움.
			LeaveCriticalSection(&g_session_map_cs); // 문제 상황
			continue;
		}
		LeaveCriticalSection(&g_session_map_cs);
		new_session->last_recv_time = timeGetTime();

		// 소켓과 입출력 완료 포트 연결 Key:session pointer
		CreateIoCompletionPort((HANDLE)client_sock, hcp, (ULONG_PTR)new_session, 0);

		// Accept RecvPost
		AcceptRecvPost_ret = AcceptRecvPost(*new_session);
		if (AcceptRecvPost_ret)
			OnClientJoin(my_session_id, *new_session);
	}
	printf("현재 AcceptThread스레드 ID:%d종료합니다\n", GetCurrentThreadId());
}

unsigned int WINAPI IOCPWorkerThread(LPVOID arg)
{
	HANDLE hcp = (HANDLE)arg;
	DWORD number_of_bytes_transfferd;
	PULONG completion_key;
	OVERLAPPED* overlapped;
	BOOL GQCS_ret;


	st_session* now_session;
	DWORD IO_count_ret;

	CSerialization packet;

	// const
	const int HEADER_SIZE = 2;

	// return
	int moveRear_ret;
	int recv_buffer_usesize_ret;

	while (!g_exit_flag)
	{
		// Init transffered and completion_key and overlapped
		number_of_bytes_transfferd = 0;
		completion_key = 0;
		overlapped = NULL; // 필요하진 않음 100% out parameter임.

		GQCS_ret = GetQueuedCompletionStatus(hcp, &number_of_bytes_transfferd, (PULONG_PTR)&completion_key, (LPOVERLAPPED*)&overlapped, INFINITE);
		// 키를 통한 종료 확인
		if (number_of_bytes_transfferd == 0 && completion_key == 0 && overlapped == nullptr)
		{
			PostQueuedCompletionStatus(hcp, 0, 0, 0);
			break;
		}

		now_session = (st_session*)completion_key;
		// client가 종료를 실행
		if (number_of_bytes_transfferd == 0)
		{
			goto iocount_down;
		}
		// overlapped 포인터가 recv라면
		if (&now_session->recv_overlapped == overlapped)
		{
			// 받은 number_of_bytes_transfferd만큼 Rear를 옮겨야 함.
			moveRear_ret = now_session->recv_buffer.MoveRear(number_of_bytes_transfferd);
			if (moveRear_ret != number_of_bytes_transfferd)
			{
				printf("Recv MoveRear[%d], numberofbytes[%d]\n", moveRear_ret, number_of_bytes_transfferd);
				Disconnect(now_session->session_id);
				continue;
			}

			recv_buffer_usesize_ret = now_session->recv_buffer.GetUseSize();
			while (recv_buffer_usesize_ret > 0)
			{
				unsigned short size;
				// network header size보다 작을 경우
				if (recv_buffer_usesize_ret < HEADER_SIZE)
				{
					break;
				}

				// packet 청소
				packet.Clear();

				// network header 관찰
				int peek_ret = now_session->recv_buffer.Peek(packet.GetBufferPtr(), HEADER_SIZE);
				if (peek_ret != HEADER_SIZE)
				{
					printf("peek-ret error\n");
					break;
				}

				// payload 사이즈 얻기
				packet >> size;

				// [size, headersize]와 recv_buffer_usesize_ret 사이즈 비교
				if (size + HEADER_SIZE > recv_buffer_usesize_ret)
				{
					break;
				}

				// header size만큼 옮기기
				now_session->recv_buffer.MoveFront(HEADER_SIZE);

				// payload를 직렬화 버퍼에 넣기.
				now_session->recv_buffer.Dequeue(packet.GetBufferPtr() + HEADER_SIZE, size );

				now_session->last_recv_time = timeGetTime();

				// OnMessage
				OnMessage(now_session->session_id, packet);

				recv_buffer_usesize_ret -= (HEADER_SIZE + size);
			}

			// 로직이 끝나서 WSARecv 걸기
			RecvPost(*now_session);
		}

		// overlapped 포인터가 send라면
		else if (&now_session->send_overlapped == overlapped)
		{
			// 완료 크기만큼 front의 pos를 바꿔준다.
			now_session->send_buffer.MoveFront(number_of_bytes_transfferd);

			// send_flag를 false로 바꿔준다.
			InterlockedExchange(&now_session->send_flag, 0); //now_session->send_flag = false; 비교해야 함.

			if (now_session->send_buffer.GetUseSize() > 0)
				SendPost(*now_session);
		}
		else
		{
			printf("IOCPWorkerThread[%p] completion_key[%p] not Send and recv\n", overlapped, completion_key);
		}

	iocount_down:
		IO_count_ret = InterlockedDecrement(&now_session->IO_count);
		if (IO_count_ret == 0)
		{
			DWORD val = now_session->session_id;
			bool flag = ReleaseSession(*now_session);
			if (flag)
				OnRelease(val);
		}
	}
	printf("현재 IOWorkerThread스레드 ID:%d종료합니다\n", GetCurrentThreadId());
	return 0;
}

extern CMemoryPool<CSerialization> g_packet_pool;
extern DWORD g_now_time;
unsigned int WINAPI EchoThread(LPVOID arg)
{
	bool flag = true; // TODO: 종료 키가 눌렸을 때 
					  //       EchoThread가 꺼질 수 있도록 고쳐야함
	short type;
	long nowSize;
	g_now_time = timeGetTime();
	while (flag)
	{
		WaitForSingleObject(g_job_event, INFINITE);
		nowSize = g_job_queue.GetUseSize();

		while (nowSize > 0)
		{
			CSerialization *recv_packet;
			CSerialization send_packet;
			DWORD session_id;

			EnterCriticalSection(&g_job_cs);
			char* packet_ptr;
			
			int ret = g_job_queue.Dequeue((char *)&packet_ptr, 8);
			if (ret != 8)
			{
				printf("Update 문제 발생\n");
			}
			recv_packet = (CSerialization *)packet_ptr;
			LeaveCriticalSection(&g_job_cs);

			*recv_packet >> session_id;
			*recv_packet >> type;

			switch (type)
			{
			case dfPACKET_CS_CLIENT_JOIN:
			{
				PacketProc_ClientJoin(session_id, *recv_packet);
				break;
			}
			case dfPACKET_CS_CLIENT_LEAVE:
			{
				PacketProc_ClientLeave(session_id);
				break;
			}
			case dfPACKET_CS_MOVE_START:
			{
				PacketProc_MoveStartPacket(session_id, *recv_packet);
				break;
			}
			case dfPACKET_CS_MOVE_STOP:
			{
				PacketProc_MoveStopPacket(session_id, *recv_packet);
				break;
			}
			case dfPACKET_CS_ATTACK1:
			{
				PacketProc_Attack1Packet(session_id, *recv_packet);
				break;
			}
			case dfPACKET_CS_ATTACK2:
			{
				PacketProc_Attack2Packet(session_id, *recv_packet);
				break;
			}
			case dfPACKET_CS_ATTACK3:
			{
				PacketProc_Attack3Packet(session_id, *recv_packet);
				break;
			}
			case dfPACKET_CS_ECHO:
			{
				PacketProc_Echo(session_id, *recv_packet);
				break;
			}
			default:
				break;
			}
			g_packet_pool.Free(recv_packet);
			nowSize -= ret;
		}

		Update();

	}
	printf("현재 EchoThread스레드 ID:%d종료합니다\n", GetCurrentThreadId());
	return 0;
}