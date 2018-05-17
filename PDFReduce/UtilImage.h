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

		
		UINT8* SerializeBitmap(HBITMAP hbmp, UINT64 *bmpSizeOut);

		//把PDF中图片点阵流数据组装成bitmap
		UINT8* AssembleBitmap(UINT8* bmpData,UINT32 uLen, INT32 width, INT32 height,UINT32 uBpc, UINT64* bmpSizeOut);
	}
}
