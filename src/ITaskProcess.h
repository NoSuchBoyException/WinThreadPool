/**********************************************************
  
   *描    述：任务处理抽象类
   *作    用：提供线程的任务处理接口
   *注意事项：无

**********************************************************/

#ifndef _ITASKPROCESS_H_
#define _ITASKPROCESS_H_

class ITaskProcess
{
public:
	virtual ~ITaskProcess(){}

	//任务处理接口
	virtual void TaskPorcess(void *pTask) = 0;
};

#endif