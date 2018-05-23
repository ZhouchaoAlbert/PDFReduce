#include "stdafx.h"
#include "ThreadManager.h"
#include <sstream>
#include <process.h>

namespace Thread{

CThreadManager::CThreadManager()
{
}

CThreadManager::~CThreadManager()
{
}

// 获取单例对象
CThreadManager * CThreadManager::GetObj()
{
	static CThreadManager obj;
	return &obj;
}

// 创建线程
bool CThreadManager::CreateThread(CScopedRefPtr<IThreadCallback> spCallBack,
	unsigned int & uiThreadID)
{
	if (!spCallBack.get())
		return false;
	spCallBack->AddRef();
	HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadProc, (void*)spCallBack, 0, &uiThreadID);
	if (hThread == NULL)
		return false;

	CScopedRefPtr<CThreadInfo>	spThreadInfo;
	spThreadInfo.attach(new CThreadInfo());
	spThreadInfo->SetThreadID(uiThreadID);
	spThreadInfo->SetThreadHandle(hThread);
	spThreadInfo->SetThreadCallback(spCallBack);
	m_mapThreadList[uiThreadID] = spThreadInfo;

	std::ostringstream ss;
	ss << "[CThreadManager::CreateThread]" << " CurrentProcessId " << ::GetCurrentProcessId()
		<< " classname: " << spCallBack->GetClass() << " thread: " << uiThreadID << " \r\n";
	WriteDebug(ss.str());
	return true;
}

// 退出指定线程
void CThreadManager::ExistThread(unsigned int uiThreadID, bool bForced)
{
	std::map<unsigned int, CScopedRefPtr<CThreadInfo>>::iterator it = m_mapThreadList.find(uiThreadID);
	if (it == m_mapThreadList.end())
		return;
	CScopedRefPtr<CThreadInfo> spThreadInfo = it->second;
	m_mapThreadList.erase(it);
	if (!spThreadInfo.get())
		return;
	HANDLE hThread = spThreadInfo->GetThreadHandle();
	if (hThread)
	{
		if (bForced)
			::TerminateThread(hThread, 0);
		else
			::WaitForSingleObject(hThread, INFINITE);
		CloseHandle(hThread);
	}

	std::ostringstream ss;
	ss << "[CThreadManager::ExistThread]" << " CurrentProcessId " << ::GetCurrentProcessId()
		<< " thread: " << spThreadInfo->GetThreadID() << " exist: " << bForced << " \r\n";
	WriteDebug(ss.str());
}

//根据线程ID拿线程回调对象
CScopedRefPtr<IThreadCallback> CThreadManager::GetCallbackByThreadID(unsigned int uiThreadID)
{
	std::map<unsigned int, CScopedRefPtr<CThreadInfo>>::iterator it = m_mapThreadList.find(uiThreadID);
	if (it == m_mapThreadList.end())
		return NULL;
	CScopedRefPtr<CThreadInfo> spThreadInfo = it->second;
	if (!spThreadInfo.get())
		return NULL;
	return spThreadInfo->GetThreadCallback();
}

//退出所有线程
void CThreadManager::ExistAll(bool bForced)
{
	std::map<unsigned int, CScopedRefPtr<CThreadInfo>>::iterator it = m_mapThreadList.begin();
	while (it != m_mapThreadList.end())
	{
		CScopedRefPtr<CThreadInfo> spThreadInfo = it->second;
		it = m_mapThreadList.erase(it);

		if (!spThreadInfo.get())
			continue;
		HANDLE hThread = spThreadInfo->GetThreadHandle();
		if (hThread)
		{
			if (bForced)
				::TerminateThread(hThread, 0);
			else
				::WaitForSingleObject(hThread, INFINITE);
			CloseHandle(hThread);
		}

		std::ostringstream ss;
		ss << "[CThreadManager::ExistAll]" << " CurrentProcessId " << ::GetCurrentProcessId()
			<< " thread: " << spThreadInfo->GetThreadID() << " exist: " << bForced << " \r\n";
		WriteDebug(ss.str());
	}
}

//////////////////////////////////////////////////////////////////////////
// 线程回调
UINT WINAPI  CThreadManager::ThreadProc(void* pVoid)
{
	CScopedRefPtr<IThreadCallback> spCallback;
	IThreadCallback *pThreadCallback = static_cast<IThreadCallback *>(pVoid);
	
	spCallback.attach(pThreadCallback);
	if (spCallback.get())
	{
		spCallback->Run();
	}
	return 0;
}

// 写打印信息
void CThreadManager::WriteDebug(std::string strDebug)
{
	OutputDebugStringA(strDebug.c_str());
}


void  GetBaseThreadObject(void **ppObject)
{
	*ppObject = static_cast<Thread::IThreadManager*>(Thread::CThreadManager::GetObj());
}

}