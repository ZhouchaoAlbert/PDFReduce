#ifndef __THREAD_MANAGER_H__
#define __THREAD_MANAGER_H__

#include <windows.h>
#include <map>
#include <string>
#include "..\Base\BaseCom.h"

namespace Thread{

// 线程信息
class CThreadInfo : public ISmartRef
{
public:
	CThreadInfo() : m_hThread(NULL), m_uiThreadID(0), m_ulRef(1){}

	unsigned int GetThreadID(){ return m_uiThreadID; }

	void SetThreadID(unsigned int uiThreadID){ m_uiThreadID = uiThreadID; }

	HANDLE GetThreadHandle(){ return m_hThread; }

	void SetThreadHandle(HANDLE hThread) { m_hThread = hThread; }

	CScopedRefPtr<IThreadCallback> GetThreadCallback() { return m_spCallback; }

	void SetThreadCallback(CScopedRefPtr<IThreadCallback> spCallback) { m_spCallback = spCallback; }
private:
	HANDLE							m_hThread;
	unsigned int					m_uiThreadID;
	CScopedRefPtr<IThreadCallback>	m_spCallback;
//
	IMPLEMENT_REFCOUNT(CThreadInfo);
};

//
// 线程管理
//
class CThreadManager : public IThreadManager
{
protected:
	CThreadManager();
public:
	virtual ~CThreadManager();
public:
	// 获取单例对象
	static CThreadManager * GetObj();

	virtual bool CreateThread(CScopedRefPtr<IThreadCallback> spCallBack,
		unsigned int & uiThreadID);
	virtual void ExistThread(unsigned int uiThreadID, bool bForced);

	virtual CScopedRefPtr<IThreadCallback> GetCallbackByThreadID(unsigned int uiThreadID);

	virtual void ExistAll(bool bForced);
protected:
	// 线程回调
	static UINT WINAPI  ThreadProc(void* pVoid);

	// 写打印信息
	void WriteDebug(std::string strDebug);
protected:
	std::map<unsigned int, CScopedRefPtr<CThreadInfo>>		m_mapThreadList;
};


void  GetBaseThreadObject(void **ppObject);

}

#endif

