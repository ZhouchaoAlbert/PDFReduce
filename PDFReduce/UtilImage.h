#pragma once
#include <tchar.h>
#include <atlstr.h>

namespace Util
{
	namespace Image
	{
		//找扩展名
		LPTSTR FindExtension(LPCTSTR szExeName);
		//图片类型反转
		BOOL ConvertType(LPCTSTR szImagePathIn, LPCTSTR szImagePathOut, UINT32 bQuality = 50);
	}
}
