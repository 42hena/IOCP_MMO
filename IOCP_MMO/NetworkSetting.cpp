#pragma comment(lib, "ws2_32")
#include <Winsock2.h>
#include <iostream>
#include <Windows.h>

// global listen socket
SOCKET g_listen_sock;

// handle important info [IP, PORT]
void InitSockAddr(SOCKADDR_IN* addr, DWORD ip_addr, WORD port)
{
	addr->sin_family = AF_INET;
	addr->sin_addr.s_addr = ip_addr;
	addr->sin_port = htons(port);
}

bool NetworkSetting()
{
	// address variable
	SOCKADDR_IN addr;

	// Window API function return variables
	int bind_ret;
	int ioctlsocket_ret;
	int setsockopt_ret;
	int listen_ret;

	// linger opt
	LINGER _linger;
	u_long blockingMode = 0;
	u_long nonBlockingMode = 1;

/*
* code start
*/

	g_listen_sock = INVALID_SOCKET;

	g_listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (g_listen_sock == INVALID_SOCKET)
	{
		wprintf(L"[ServerOn] -> socket() error\n");
		return false;
	}

	ZeroMemory(&addr, sizeof(addr));
	InitSockAddr(&addr, INADDR_ANY, 10801);

	bind_ret = bind(g_listen_sock, (SOCKADDR*)&addr, sizeof(addr));
	if (bind_ret == SOCKET_ERROR)
	{
		wprintf(L"[ServerOn] -> bind() error\n");
		return false;
	}

	ioctlsocket_ret = ioctlsocket(g_listen_sock, FIONBIO, &nonBlockingMode);
	if (ioctlsocket_ret == SOCKET_ERROR)
	{
		wprintf(L"[ServerOn] -> ioctlsocket() error\n");
		return false;
	}

	_linger.l_onoff = 1;
	_linger.l_linger = 0;

	setsockopt_ret = setsockopt(g_listen_sock, SOL_SOCKET, SO_LINGER, (char*)&_linger, sizeof(_linger));
	if (setsockopt_ret == SOCKET_ERROR)
	{
		printf("[ServerOn] -> setsockopt() error\n");
		return false;
	}

	listen_ret = listen(g_listen_sock, SOMAXCONN_HINT(65535));
	if (listen_ret == SOCKET_ERROR)
	{
		printf("[ServerOn] -> listen() error\n");
		return false;
	}
	return true;
}
