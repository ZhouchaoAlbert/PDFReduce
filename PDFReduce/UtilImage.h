#pragma once
#include <tchar.h>
#include <atlstr.h>

namespace Util
{
	namespace Image
	{
		//����չ��
		CString FindExtension(ATL::CString  strExeName);
		//ͼƬ���ͷ�ת
		BOOL ConvertType(ATL::CString strImagePathIn, ATL::CString  strImagePathOut, UINT32 bQuality = 50);
	}
}
