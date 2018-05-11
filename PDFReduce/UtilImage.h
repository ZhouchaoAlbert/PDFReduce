#pragma once
#include <tchar.h>
#include <atlstr.h>

namespace Util
{
	namespace Image
	{
		//找扩展名
		CString FindExtension(ATL::CString  strExeName);
		//图片类型反转
		BOOL ConvertType(ATL::CString strImagePathIn, ATL::CString  strImagePathOut, UINT32 bQuality = 50);
	}
}
