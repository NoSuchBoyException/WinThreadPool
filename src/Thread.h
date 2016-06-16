/**********************************************************

   *描    述：工作线程类 派生于IThread
   *作    用：接受并执行用户任务
   *注意事项：可从指针型任务和对象型任务中任选一种作为任务载体

**********************************************************/

#ifndef _THREAD_H_
#define _THREAD_H_

#include "IThread.h"
#include "ITaskProcess.h"

class Thread : public IThread
{
public:
	//接受指针型任务
	Thread(void (*pTaskProcessFunc)(void *), void *pTask = NULL);
	//接受对象型任务
    Thread(ITaskProcess *pTaskProcess, void *pTask = NULL);

protected:
	//实现IThread的Run方法
	virtual void Run();

private:
	//任务类型判断标识
	bool m_bIsObjTask;

	//对象型任务单元
	struct ObjTaskItem
	{
		//任务处理对象
		ITaskProcess *m_pTaskProcess;

		//任务数据
		void *m_pTask;

	} m_objTaskItem;

	//指针型任务单元
	struct PtrTaskItem
	{
		//任务处理函数
		void (*m_pTaskProcessFunc)(void *);

		//任务数据
		void *m_pTask; 

	} m_ptrTaskItem;
};

#endif