#pragma once
#include <tchar.h>
#include <atlstr.h>

namespace Util
{
	namespace Image
	{
		//����չ��
		LPTSTR FindExtension(LPCTSTR szExeName);
		//ͼƬ���ͷ�ת
		BOOL ConvertType(LPCTSTR szImagePathIn, LPCTSTR szImagePathOut, UINT32 bQuality = 50);
	}
}
