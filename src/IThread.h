/**********************************************************

   *描    述：线程基类
   *作    用：提供线程的基本操作
   *注意事项：析构本类对象前须释放Run()函数内申请的资源

**********************************************************/

#ifndef _ITHREAD_H_
#define _ITHREAD_H_

#include <windows.h>

class IThread
{
public:
	IThread();
	virtual ~IThread();

	//启动线程的唯一方法
    bool Start();

	//获取线程ID
	DWORD GetThreadID() const;

	//获取线程HANDLE
	HANDLE GetThreadHandle() const;

protected:
	//子类需要实现该方法以完成业务处理
	virtual void Run() = 0;

private:
	//线程函数
	static DWORD WINAPI ThreadFunc(LPVOID lpParam);

private:
	//线程ID
	DWORD m_dwThreadID;

	//线程句柄
	HANDLE m_hThreadHandle;
};

#endif