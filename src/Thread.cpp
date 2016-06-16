#include "Thread.h"

Thread::Thread(void (*pTaskProcessFunc)(void *), void *pTask)
{
	m_ptrTaskItem.m_pTaskProcessFunc = pTaskProcessFunc;
	m_ptrTaskItem.m_pTask = pTask;

	m_bIsObjTask = false;
}

Thread::Thread(ITaskProcess *pTaskProcess, void *pTask)
{
	m_objTaskItem.m_pTaskProcess = pTaskProcess;
	m_objTaskItem.m_pTask = pTask;

	m_bIsObjTask = true;
}

void Thread::Run()
{
	if (true == m_bIsObjTask)
	{
		if (NULL != m_objTaskItem.m_pTaskProcess)
		{
			//执行任务处理
			m_objTaskItem.m_pTaskProcess->TaskPorcess(m_objTaskItem.m_pTask);
		}
	}
	else
	{
		if (NULL != m_ptrTaskItem.m_pTaskProcessFunc)
		{
			//执行任务处理
			m_ptrTaskItem.m_pTaskProcessFunc(m_ptrTaskItem.m_pTask);
		}
	}
}