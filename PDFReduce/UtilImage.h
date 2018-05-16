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

		
		UINT8* SerializeBitmap(HBITMAP hbmp, UINT64 *bmpSizeOut);
		UINT8* CreateRenderedBitmap(const char *bmpData, LONG dx, LONG dy, bool grayscale,UINT64 *bmpBytesOut);

		//��PDF��ͼƬ������������װ��bitmap
		UINT8* AssembleBitmap(UINT8* bmpData, INT32 width, INT32 height, BOOL bGrayScale, UINT64* bmpSizeOut);
	}
}
