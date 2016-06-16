/**********************************************************

   *描    述：线程池管理器
   *作    用：实现对工作线程的统一管理
   *注意事项：1.本管理器通过单例模式实现
              2.本线程池可跨进程使用
              3.初始、最小和最大工作线程数分别为512、1及2048

**********************************************************/

#ifndef _THREADPOOLMANAGER_H_
#define _THREADPOOLMANAGER_H_

#include <vector>
#include <assert.h>
#include <iostream>
#include "TaskQueueManager.h"
#include "Thread.h"

using namespace std;

class ThreadPoolManager
{
public:
	//调用线程池管理器的唯一方法
	static ThreadPoolManager *GetInstance();

	~ThreadPoolManager();

	//执行任务处理
	void ExecuteTaskProcess();

	//设置线程池中的工作线程数
	void SetThreadCount(const size_t &zThreadCount);

	//获取线程池中的工作线程数
	size_t GetThreadCount() const;

	//获取线程池中的活跃工作线程数
	size_t GetActiveThreadCount() const;

private:
	ThreadPoolManager();

	//线程工作函数
	static void ThreadWorkFunc(void *pParam);

private:
	//线程池管理器单例
	static ThreadPoolManager *ms_pInstance;

	//任务队列单例
	TaskQueueManager *m_pTaskQueueManager;

	//线程池及其互斥锁
	vector<IThread *> m_ThreadPool;
	HANDLE m_mutexThreadPool;

	//线程池中的工作线程数
	size_t m_zThreadCount;

	//线程池中的活跃工作线程数
	size_t m_zActiveThreadCount;

	//执行任务处理信号量信号
	HANDLE m_semExecuteTaskProcess;

	//删除工作线程信号量信号
	HANDLE m_semDeleteThread;

	//工作线程删除完成事件信号
	HANDLE m_evtDeleteThreadComplete;

	//卸载线程池事件信号
	HANDLE m_evtUninitPool;

	//线程池卸载完成事件信号
	HANDLE m_evtUninitPoolComplete;

	//线程池的初始工作线程数
	static const int INITIAL_THREAD_COUNT = 512;

	//线程池的最小工作线程数
	static const int MIN_THREAD_COUNT = 1;

	//线程池的最大工作线程数
	static const int MAX_THREAD_COUNT = 2048;
};

#endif