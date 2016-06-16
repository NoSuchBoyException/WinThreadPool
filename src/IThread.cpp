#include "IThread.h"

IThread::IThread()
{
	m_dwThreadID = -1;
	m_hThreadHandle = NULL;
}

IThread::~IThread()
{
	if (NULL != m_hThreadHandle)
	{
		//释放线程内核对象 防止内核泄露
		CloseHandle(m_hThreadHandle);
		m_hThreadHandle = NULL;
	}
}

bool IThread::Start()
{
	m_hThreadHandle = CreateThread(NULL, 0, ThreadFunc, 
                                   this, 0, &m_dwThreadID);
	if (NULL == m_hThreadHandle)
	{
		m_dwThreadID = -1;

		return false;
	}
	else
	{
		return true;
	}
}

DWORD IThread::GetThreadID() const
{
	return m_dwThreadID;
}

HANDLE IThread::GetThreadHandle() const
{
	return m_hThreadHandle;
}

DWORD WINAPI IThread::ThreadFunc(LPVOID lpParam)
{
	if (NULL == lpParam)
	{
		return -1;
	}

	//执行业务逻辑
	((IThread *)(lpParam))->Run();

	return 0;
}