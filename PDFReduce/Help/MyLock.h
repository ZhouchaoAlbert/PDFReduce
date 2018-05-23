#ifndef __MYLOCAK_H__
#define __MYLOCAK_H__

#include <WinBase.h>

class  CCritSec
{
public:
	CCritSec() {
		InitializeCriticalSection(&m_CritSec);
	}

	virtual ~CCritSec() {
		DeleteCriticalSection(&m_CritSec);
	}
public:
	// ¼ÓËø
	void Lock() {
		EnterCriticalSection(&m_CritSec);
	}

	// ½âËø
	void Unlock() {
		LeaveCriticalSection(&m_CritSec);
	}
protected:
	CRITICAL_SECTION m_CritSec;
};
#endif