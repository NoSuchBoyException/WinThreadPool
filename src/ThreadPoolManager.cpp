#include "ThreadPoolManager.h"

ThreadPoolManager *ThreadPoolManager::ms_pInstance = NULL;

ThreadPoolManager *ThreadPoolManager::GetInstance()
{
	if (NULL == ms_pInstance)
	{
		ms_pInstance = new ThreadPoolManager;
	}

	return ms_pInstance;
}

ThreadPoolManager::ThreadPoolManager()
{
	/*
      * 函数名：CreateMutex
	  * 功能：  创建并初始化互斥锁内核对象
	  * 参数：  lpMutexAttributes   安全属性指针        一般为NULL
	  *         bInitialOwner       是否为创建进程拥有  一般为FALSE
	  *         lpName              内核对象名称        可为NULL
	  * 返回值：成功时返回互斥锁内核对象句柄 失败时返回NULL
	*/
	m_mutexThreadPool = CreateMutex(NULL, FALSE, NULL);
	assert(NULL != m_mutexThreadPool);

	/*
	  * 函数名：CreateEvent
	  * 功能：  创建并初始化事件内核对象
	  * 参数：  lpEventAttributes   安全属性指针        一般为NULL
	  *         bManualReset        TRUE人工重置        FALSE自动重置
	  *         bInitialState       TRUE有信号状态      FALSE无信号状态
	  *         lpName              内核对象名称        可为NULL
	  * 返回值：成功时返回事件内核对象句柄 失败时返回NULL
	*/
	m_evtDeleteThreadComplete = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(NULL != m_evtDeleteThreadComplete);

	m_evtUninitPool = CreateEvent(NULL, TRUE, FALSE, NULL);
	assert(NULL != m_evtUninitPool);

	m_evtUninitPoolComplete = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(NULL != m_evtUninitPoolComplete);

	/*
	  * 函数名：CreateSemaphore
	  * 功能：  创建并初始化信号量内核对象
	  * 参数：  lpSemaphoreAttributes   安全属性指针     一般为NULL
	  *         lInitialCount           初始资源计数
	  *         lMaximumCount           允许最大资源计数
	  *         lpName                  内核对象名称     可为NULL
	  * 返回值：成功时返回信号量内核对象句柄 失败时返回NULL
	*/
	m_semExecuteTaskProcess = CreateSemaphore(NULL, 0, 0x7FFFFFFF, NULL);
	assert(NULL != m_semExecuteTaskProcess);

	m_semDeleteThread = CreateSemaphore(NULL, 0, 0x7FFFFFFF, NULL);
	assert(NULL != m_semDeleteThread);

	//获取任务队列管理器单例
	m_pTaskQueueManager = TaskQueueManager::GetInstance();

	//向线程池添加 INITIAL_THREAD_COUNT 数量的工作线程
	SetThreadCount(INITIAL_THREAD_COUNT);

	//初始化工作线程计数
	m_zThreadCount = INITIAL_THREAD_COUNT;
}

ThreadPoolManager::~ThreadPoolManager()
{
	//等待队列中的任务被全部处理
	while (true)
	{
		m_pTaskQueueManager->Lock();
		if (0 != m_pTaskQueueManager->Size())  //还有任务未处理
		{
			m_pTaskQueueManager->Unlock();

	        //休眠5毫秒继续执行判断
			Sleep(5);
			continue;
		}
		else                                   //任务已全部被处理
		{
			m_pTaskQueueManager->Unlock();
			break;
		}
	}

	//设置卸载线程池事件信号
	SetEvent(m_evtUninitPool);

	//等待线程池卸载完成事件信号
	WaitForSingleObject(m_evtUninitPoolComplete, INFINITE);

	//释放内核对象 防止内核泄露
	CloseHandle(m_mutexThreadPool);
	m_mutexThreadPool = NULL;

	CloseHandle(m_semExecuteTaskProcess);
	m_semExecuteTaskProcess = NULL;

	CloseHandle(m_semDeleteThread);
	m_semDeleteThread = NULL;

	CloseHandle(m_evtDeleteThreadComplete);
	m_evtDeleteThreadComplete = NULL;

	CloseHandle(m_evtUninitPool);
	m_evtUninitPool = NULL;

	CloseHandle(m_evtUninitPoolComplete);
	m_evtUninitPoolComplete = NULL;
	
	//析构任务队列管理器单例
	delete m_pTaskQueueManager;
}

void ThreadPoolManager::ExecuteTaskProcess()
{
	//释放1个执行任务处理信号量信号
	ReleaseSemaphore(m_semExecuteTaskProcess, 1, NULL);
}

void ThreadPoolManager::SetThreadCount(const size_t &zThreadCount)
{
	//保证线程池中的工作线程数在最小线程数与最大线程数之间
	if (MIN_THREAD_COUNT>zThreadCount || MAX_THREAD_COUNT<zThreadCount)
	{
		return;
	}

	//设置减少工作线程数标识
	bool bIsDecreaseThreadCount = false;

	WaitForSingleObject(m_mutexThreadPool, INFINITE);
	//更新工作线程计数
	m_zThreadCount = zThreadCount;
	//当前工作线程数
	size_t zCurrentThreadCount = m_ThreadPool.size();
	//当前工作线程数小于设定线程数
	if (zThreadCount > zCurrentThreadCount)
	{
		//向线程池添加 zThreadCountIncrement 数量的工作线程
		size_t zThreadCountIncrement = zThreadCount - zCurrentThreadCount;
		IThread *pWorkerThread = NULL;
		for (size_t i=0; i<zThreadCountIncrement; ++i)
		{
			//创建并启动工作线程
			pWorkerThread = new Thread(ThreadWorkFunc, (void *)this);
			pWorkerThread->Start();
			//向线程池添加工作线程
			m_ThreadPool.push_back(pWorkerThread);
		}
	}
	//当前工作线程数大于设定线程数
	else if (zThreadCount < zCurrentThreadCount)
	{
		//设置减少工作线程数标识
		bIsDecreaseThreadCount = true;
		//释放 zThreadCountIncrement 数量的删除工作线程信号量信号
		size_t zThreadCountIncrement = zCurrentThreadCount - zThreadCount;
		ReleaseSemaphore(m_semDeleteThread, zThreadCountIncrement, NULL);
	}
	ReleaseMutex(m_mutexThreadPool);

	if (true == bIsDecreaseThreadCount)
	{
		//等待 zThreadCountIncrement 数量的工作线程删除完成
		WaitForSingleObject(m_evtDeleteThreadComplete, INFINITE);
	}
}

size_t ThreadPoolManager::GetThreadCount() const
{
	WaitForSingleObject(m_mutexThreadPool, INFINITE);
	size_t zThreadCount = m_ThreadPool.size();
	ReleaseMutex(m_mutexThreadPool);

	return zThreadCount;
}

size_t ThreadPoolManager::GetActiveThreadCount() const
{
	return m_zActiveThreadCount;
}

void ThreadPoolManager::ThreadWorkFunc(void *pParam)
{
	if (NULL == pParam)
	{
		return;
	}

	ThreadPoolManager *pThisManager = static_cast<ThreadPoolManager *>(pParam);

	//信号等待数组
	HANDLE arrSingalWait[3] = { pThisManager->m_semExecuteTaskProcess, 
								pThisManager->m_semDeleteThread, 
								pThisManager->m_evtUninitPool };
	bool bHasTask = false;
	bool bIsUninitPool = false;
	DWORD dwSingalValue = 0;
	TaskItem *pTaskItem = NULL;
	while (true)
	{
		//等待arrSingalWait中的信号
		dwSingalValue = WaitForMultipleObjects(3, arrSingalWait, 
	                                           false, INFINITE);
		//收到执行任务处理信号量信号
		if (0 == dwSingalValue-WAIT_OBJECT_0)
		{
			//从任务队列取出并Pop掉最顶端的任务
			pThisManager->m_pTaskQueueManager->Lock();
			bHasTask = !pThisManager->m_pTaskQueueManager->IsEmpty();
			if (true == bHasTask)
			{
				//取出任务队列最顶端的任务
				pTaskItem = pThisManager->m_pTaskQueueManager->Front();
				//Pop掉任务队列最顶端的任务
				pThisManager->m_pTaskQueueManager->PopFront();
			}
			pThisManager->m_pTaskQueueManager->Unlock();

			//处理并析构取出的任务
			InterlockedIncrement(&pThisManager->m_zActiveThreadCount);
			if (true==bHasTask && NULL!=pTaskItem)
			{
				//执行任务处理
				pTaskItem->m_pTaskProcessFunc(pTaskItem->m_pTask);
				//析构任务
				delete pTaskItem;
				pTaskItem = NULL;
			}
			InterlockedDecrement(&pThisManager->m_zActiveThreadCount);
		}
		//收到删除工作线程信号量信号
		else if (1 == dwSingalValue-WAIT_OBJECT_0)
		{
			bIsUninitPool = false;
			break;
		}
		//收到卸载线程池事件信号
		else if (2 == dwSingalValue-WAIT_OBJECT_0)
		{
			bIsUninitPool = true;
			break;
		}
	}

	//从线程池删除该工作线程
	WaitForSingleObject(pThisManager->m_mutexThreadPool, INFINITE);
	if (false == pThisManager->m_ThreadPool.empty())
	{
		vector<IThread *>::iterator iter = pThisManager->m_ThreadPool.begin();
		for (; iter!=pThisManager->m_ThreadPool.end(); ++iter)
		{
			if ((*iter)->GetThreadID() == GetCurrentThreadId())
			{
				//析构该工作线程
				delete static_cast<IThread *>(*iter);
				//从线程池擦除该工作线程
				pThisManager->m_ThreadPool.erase(iter);
				break;
			}
		}
		//判断是否删除完所有线程
		if (true == bIsUninitPool)
		{
			if (0 == pThisManager->m_ThreadPool.size())
			{
				//设置线程池卸载完成事件信号
				SetEvent(pThisManager->m_evtUninitPoolComplete);
			}
		}
		//判断是否删除完指定数量的线程
		else
		{
			if (pThisManager->m_zThreadCount == 
				pThisManager->m_ThreadPool.size())
			{
				//设置线程数设置完成事件信号
				SetEvent(pThisManager->m_evtDeleteThreadComplete);
			}
		}
	}
	ReleaseMutex(pThisManager->m_mutexThreadPool);
}