#ifndef __THREAD_Info_H__
#define __THREAD_Info_H__

/*
* Accept�� �����ϴ� ������
* Parameters:	(LPVOID)
* return:		(unsigned int)
*/
unsigned int WINAPI AcceptThread(LPVOID arg);


/*
* Accept�� �����ϴ� ������
* Parameters:	(LPVOID)
* return:		(unsigned int)
*/
unsigned int WINAPI IOCPWorkerThread(LPVOID arg);

/*
*
*/
unsigned int WINAPI EchoThread(LPVOID arg);

#endif
