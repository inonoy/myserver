#include "EasyTcpClient.hpp"
#include<thread>
#include<atomic>
#include<iostream>
#define CONTINUE_MSG_COUNT 1
bool g_bRun = true;
void cmdThread()
{
	while (true)
	{
		char cmdBuf[256] = {};
		scanf_s("%s", cmdBuf);
		//std::cin >> cmdBuf;
		if (0 == strcmp(cmdBuf, "exit"))
		{
			g_bRun = false;
			printf("退出cmdThread线程\n");
			break;
		}
		else {
			printf("不支持的命令。\n");
		}
	}
}

//客户端数量
const int cCount = 1000;
//发送线程数量
const int tCount = 4;
//客户端数组
EasyTcpClient* client[cCount];

std::atomic_int _sendCount = 0;
std::atomic_int _readyCount = 0;
void sendThread(int id)
{
	printf("thread<%d>,start\n", id);
	//4个线程 ID 1~4
	int c = cCount / tCount;
	int begin = (id - 1) * c;
	int end = id * c;

	for (int n = begin; n < end; n++)
	{
		client[n] = new EasyTcpClient();
	}
	for (int n = begin; n < end; n++)
	{
		client[n]->Connect("127.0.0.1", 4567);
	}

	printf("thread<%d>,Connect<begin=%d, end=%d>\n", id, begin, end);
	_readyCount++;
	std::chrono::milliseconds t(10);
	//当所有的线程准备就绪才开始收发数据
	while (_readyCount < tCount) {

		std::this_thread::sleep_for(t);
	}

	Login login[CONTINUE_MSG_COUNT];
	for (int n = 0; n < CONTINUE_MSG_COUNT; n++)
	{
#ifdef _WIN32
		strcpy_s(login[n].username, "lyd");
		strcpy_s(login[n].password, "lydmm");

#else
		strcpy(login[n].username, "lyd");
		strcpy(login[n].password, "lydmm");

#endif // DEBUG


	}
	const int nLen = sizeof(login);
	while (g_bRun)
	{
		for (int n = begin; n < end; n++)
		{
			if (SOCKET_ERROR != client[n]->SendData(login, nLen)) {

				_sendCount++;
			}

		/*	std::chrono::milliseconds t(1);
			std::this_thread::sleep_for(t);
		*/
			//client[n]->OnRun();
		}
	}

	for (int n = begin; n < end; n++)
	{
		client[n]->Close();
		delete client[n];
	}

	printf("thread<%d>,exit\n", id);
}

int main()
{
	//启动UI线程
	std::thread t1(cmdThread);
	t1.detach();

	//启动发送线程
	for (int n = 0; n < tCount; n++)
	{
		std::thread t1(sendThread, n + 1);
		t1.detach();
	}
	CELLTimestamp tTime;
	//std::chrono::milliseconds t(10);

	while (g_bRun) {
		auto t = tTime.getElapsedSecond();
		if (t > 1.0) {
			printf("thread<%d>,client<%d>,time<%lf>,send<%d>\n", tCount, cCount, t, (int)_sendCount);
			tTime.update();
			_sendCount = 0;
		}
		Sleep(1);
	}
	printf("已退出。\n");
	return 0;
}