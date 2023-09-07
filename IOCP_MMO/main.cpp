#pragma comment(lib, "winmm.lib")
#include <iostream>
#include <process.h>
#include <Windows.h>
#include <unordered_map>
#include <list>


#include "ThreadInfo.h"
#include "NetworkSetting.h"
#include "CRingBuffer.h"
#include "session.h"
#include "PACKET_PROTOCOL.h"
#include "CSerializationBuffer.h"

// define 정보
#define WORKER_THREAD_COUNT 2
using namespace std;
extern HANDLE g_job_event;
extern CRITICAL_SECTION g_job_cs;
extern CRITICAL_SECTION g_session_map_cs;

list< st_Character* > g_sector[dfSECTOR_MAX_Y + 1][dfSECTOR_MAX_X + 1];
unordered_map<DWORD, st_Character*> g_character_map;


bool CreateIOCP(HANDLE &hcp, HANDLE hWorkerThreads[])
{
	wprintf(L"Make IOCP and WorkerThreads\n");

	// 입출력 완료 포트 생성
	hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (hcp == NULL)
	{
		printf("입출력 포트 생성 실패[%d]\n", GetLastError());
		return false;
	}

	for (int i = 0; i < WORKER_THREAD_COUNT; ++i)
	{
		hWorkerThreads[i] = (HANDLE)_beginthreadex(NULL, 0, IOCPWorkerThread, hcp, 0, NULL);
		if (hWorkerThreads[i] == NULL)
		{
			printf("_beginthreadex Error[%d]\n", GetLastError());
			return false;
		}
	}

	return true;
}

bool CreateUpdateThread(HANDLE hcp, HANDLE& hUpdateThread)
{
	wprintf(L"Make UpdateThreads\n");

	// UpdateThraed 생성
	hUpdateThread = (HANDLE)_beginthreadex(NULL, 0, EchoThread, NULL, 0, NULL); // TODO:Echo thread에서 바꿔야 함.
	if (hUpdateThread == nullptr)
		return false;
	return true;
}

bool CreateAcceptThread(HANDLE hcp, HANDLE &hAcceptThread)
{
	wprintf(L"Make AcceptThreads\n");

	// AcceptThraed 생성
	hAcceptThread = (HANDLE)_beginthreadex(NULL, 0, AcceptThread, hcp, 0, NULL);
	if (hAcceptThread == nullptr)
		return false;
	return true;
}
#include <conio.h>
#include "session.h"
#include "CMemoryPool.h"

extern CRingBuffer g_job_queue;
extern CMemoryPool<st_session> g_session_pool;
extern CMemoryPool<CSerialization> g_packet_pool;
extern unordered_map<DWORD, st_Character*> g_character_map;
void Run()
{
	bool setting_ret;
	
	// Threads variable
	HANDLE hAcceptThread;
	HANDLE hUpdateThread;
	bool thread_ret;

	// IOCP
	HANDLE hcp;
	HANDLE hWorkerThreads[WORKER_THREAD_COUNT];
	bool IOCP_ret;

	wprintf(L"Start setting right before server operation\n");
	setting_ret = NetworkSetting();
	if (setting_ret == false)
	{
		printf("NetworkSetting function Error occured\n");
		return ;
	}

	// Create IOCP
	IOCP_ret = CreateIOCP(hcp, hWorkerThreads);
	if (IOCP_ret == false)
		return ;

	wprintf(L"Make threads\n");
	// Make EchoThread
	thread_ret = CreateUpdateThread(hcp, hUpdateThread);
	if (thread_ret == false)
		return;

	// Make Accept Thread
	thread_ret = CreateAcceptThread(hcp, hAcceptThread);
	if (thread_ret == false)
		return;

	// job event 생성
	g_job_event = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (g_job_event == NULL)
	{
		printf("Job event 만들기 실패\n");
		return ;
	}

	// CRITICAL_SECTION 초기화
	InitializeCriticalSection(&g_job_cs);
	InitializeCriticalSection(&g_session_map_cs);

	int i = 0;
	while (1)
	{
		SetEvent(g_job_event);
		Sleep(40);
		if (_kbhit())
		{
			WCHAR control_key = _getwch();
		}
		/*++i;
		if (i == 25)
		{
			i = 0;
			printf("--------------------------------------\n");
			printf("Session total Count : %d Use Count[%d]\n", g_session_pool.GetAllocCount(), g_session_pool.GetUseCount());
			printf("Character total Count : %I64u\n", g_character_map.size());
			printf("pool 전체 개수[%d] 사용량[%d]\n", g_packet_pool.GetAllocCount(), g_packet_pool.GetUseCount());
			printf("Job Queue 전체 개수[%d] 사용량[%d]\n", g_job_queue.GetBufferSize(), g_job_queue.GetUseSize());
			printf("--------------------------------------\n\n");
		}*/
	}


	wprintf(L"Server shut down normally.\n");
}
unordered_map<int, int> g_t;
int main()
{
	timeBeginPeriod(1);

	// Windows Socket API variable
	WSADATA wsa;


	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		wprintf(L"[main] -> WSAStartup()\n");
		return 1;
	}
	// Program main Logic
	Run();

	if (WSACleanup() != 0)
	{
		wprintf(L"[main] -> WSACleanup()\n");
	}

	timeEndPeriod(1);
	return 0;
}
