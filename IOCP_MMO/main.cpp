#pragma comment(lib, "winmm.lib")
#include <iostream>
#include <Windows.h>

#include "NetworkSetting.h"

int main()
{
	// Windows Socket API variable
	WSADATA wsa;
	bool setting_ret;

	timeBeginPeriod(1);
	
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		wprintf(L"[main] -> WSAStartup()\n");
		return false;
	}

	wprintf(L"Start setting right before server operation\n");
	setting_ret = NetworkSetting();
	if (setting_ret == false)
	{
		printf("NetworkSetting function Error occured\n");
		return 1;
	}

	timeEndPeriod(1);
	
	if (WSACleanup() != 0)
	{
		wprintf(L"[main] -> WSACleanup()\n");
		return 1;
	}

	wprintf(L"Server shut down normally.\n");

	return 0;
}
