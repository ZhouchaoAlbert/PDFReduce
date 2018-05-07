#include "UtilImage.h"
#include "ximage.h"


LPTSTR Util::Image::FindExtension(LPCTSTR szExeName)
{
	int nLen = _tcslen(szExeName);
	int i;
	for (i = nLen - 1; i >= 0; i--){
		if (szExeName[i] == '.'){
			return (TCHAR*)(szExeName + i + 1);
		}
	}
	return (LPTSTR)(szExeName + nLen);
}


BOOL Util::Image::ConvertType(LPCTSTR szImagePathIn, LPCTSTR szImagePathOut, UINT32 bQuality/* = 50*/)
{
	//1.判断类型 
	TCHAR* szExtIn = FindExtension(szImagePathIn);
	_tcslwr(szExtIn);
	INT32 nTypeIn = CxImage::GetTypeIdFromName(szExtIn);
	if (nTypeIn == CXIMAGE_FORMAT_UNKNOWN) {
		MessageBox(NULL, _T("输入的图片格式不对,请重新输入!"), _T("提示"), MB_OK);
		return FALSE;
	}

	TCHAR* szExtOut = FindExtension(szImagePathOut);
	_tcslwr(szExtOut);
	INT32 nTypeOut = CxImage::GetTypeIdFromName(szExtOut);
	if (nTypeOut == CXIMAGE_FORMAT_UNKNOWN) {
		MessageBox(NULL, _T("输出的图片格式不对,请重新输入!"), _T("提示"), MB_OK);
		return FALSE;
	}

	//2对输入图片的大小分别做判断

	//3图片转换
	CxImage image;
	image.SetJpegScale(8);
	image.Load((const TCHAR*)szImagePathIn, nTypeIn);
	if (!image.IsValid()) 
	{
		MessageBox(NULL, _T("转换失败"), _T("提示"), MB_OK);
		return FALSE;
	}
	image.SetJpegQuality(bQuality);
	image.Save((const TCHAR*)szImagePathOut, nTypeOut);
	return TRUE;
}