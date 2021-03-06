#ifndef __UTIL_MSG_H__
#define __UTIL_MSG_H__

#include <Windows.h>
namespace Util
{
	namespace Msg
	{
		class  CMsgBase
		{
		public:
			CMsgBase();
			~CMsgBase();
		public:
			void Start();
			void Stop();
			void AddMsg(UINT32 uMsgID);
			void DeleteMsg(UINT32 uMsgID);
			HWND GetMsgWnd();

			virtual void OnMessage(UINT32 uMsgID, WPARAM wParam, LPARAM lParam, BOOL& bHandled) = 0;
		};
	}
}

#endif