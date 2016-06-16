#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "TaskQueueManager.h"
#include "ThreadPoolManager.h"

//工作线程的任务处理函数
void TaskProcessFunc(void *);

int main()
{
	//初始内存状态记录
	_CrtMemState s1, s2, s3;
	_CrtMemCheckpoint(&s1);

	//获取任务队列管理器单例
	TaskQueueManager *pTaskQueueManager = TaskQueueManager::GetInstance();

	//获取线程池管理器单例
	ThreadPoolManager *pThreadPoolManager = ThreadPoolManager::GetInstance();

	//向任务队列添加任务并执行任务处理
	for (int i=0; i<1024; ++i)
	{
		//向任务队列添加任务
		pTaskQueueManager->Lock();
		pTaskQueueManager->PushBack(TaskProcessFunc, NULL);
		pTaskQueueManager->Unlock();

		//执行任务处理
		pThreadPoolManager->ExecuteTaskProcess();
	}

	getchar();

	pThreadPoolManager->SetThreadCount(1024);
	cout << "Current count of worker threads in thread-pool is: " 
		 <<  pThreadPoolManager->GetThreadCount() << endl;

	//析构线程池管理器
	delete pThreadPoolManager;

	//最终内存状态记录
	_CrtMemCheckpoint(&s2);
	//内存泄露检测
	if (0 != _CrtMemDifference(&s3, &s1, &s2))
	{
		_CrtMemDumpStatistics(&s3);
	}

	return 0;
}

void TaskProcessFunc(void *)
{
	cout << "Worker thread: " << GetCurrentThreadId() 
		 << " is running"     << endl;

	//模拟算法处理
	Sleep(50);
}