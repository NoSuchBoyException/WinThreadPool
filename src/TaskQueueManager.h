/**********************************************************
  
   *描    述：任务队列管理器类
   *作    用：为线程池提供任务队列以及任务单元
   *注意事项：操作任务队列时需使用Lock()和Unlock()函数保证线程安全

**********************************************************/

#ifndef _TASKQUEUE_H_
#define _TASKQUEUE_H_

#include <Windows.h>
#include <deque>
#include "ITaskProcess.h"

using namespace std;

/** 任务单元 **/
struct TaskItem
{
	TaskItem(void (*pTaskProcessFunc)(void *), void *pTask = NULL)
	{
		m_pTaskProcessFunc = pTaskProcessFunc;
		m_pTask = pTask;
	}

	//任务处理函数
	void (*m_pTaskProcessFunc)(void *);
	
	//任务数据
	void *m_pTask;
};

/** 任务队列管理器 **/
class TaskQueueManager
{
public:
	//调用任务队列管理器的唯一方法
	static TaskQueueManager *GetInstance();

	~TaskQueueManager();

	//锁住任务队列
	void Lock();

	//解锁任务队列
	void Unlock();

	//向任务队列添加指针型任务
	void PushBack(void (*pTaskProcessFunc)(void *), void *pTask = NULL);
	//向任务队列添加对象型任务
	void PushBack(ITaskProcess *pTaskProcess, void *pTask = NULL);

	//取出任务队列最顶端的任务
	TaskItem *Front();

	//Pop掉任务队列最顶端的任务
	void PopFront();

	//获取任务队列大小
	size_t Size();

	//任务队列是否为空
	bool IsEmpty();

private:
	TaskQueueManager();

	//转换并处理对象型任务单元函数
	static void TaskConvertFunc(void *pParam);

private:
	//任务队列管理器单例
	static TaskQueueManager *ms_pInstance;

	//任务队列及其互斥锁
	deque<TaskItem *> m_TaskQueue;
	HANDLE m_mutexTaskQueue;

	//对象型任务单元
	struct ObjTaskItem
	{
		ObjTaskItem(ITaskProcess *pTaskProcess, void *pTask = NULL)
		{
			m_pTaskProcess = pTaskProcess;
			m_pTask = pTask;
		}

		//任务处理对象
		ITaskProcess *m_pTaskProcess;

		//任务数据
		void *m_pTask;
	};
};

#endif