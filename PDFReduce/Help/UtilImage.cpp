#include "UtilImage.h"
#include "ximage.h"
#include <exception>

ATL::CString Util::Image::FindExtension(ATL::CString strExeName)
{
	INT32 nFind = strExeName.ReverseFind('.');
	if (nFind < 0)
	{
		return _T("");
	}
	return strExeName.Right(strExeName.GetLength() - nFind-1);
}


void DoSplitTiff(LPCTSTR strImagePath, LPCTSTR szSavePath)
{
	CxImage image;
	int nFrameCnt = 0;

	//加载图片,给image结构赋值
	image.Load(strImagePath, CXIMAGE_FORMAT_TIF);

	//计算一张tiff图片是由多少张图片合成的
	nFrameCnt = image.GetNumFrames();

	//每次循环都加载1张tiff图片，并保存到m_pImageData中
	for (int nIndex = 0; nIndex < nFrameCnt; nIndex++)
	{
		image.SetFrame(nIndex);
		image.Load(strImagePath, CXIMAGE_FORMAT_TIF);

		//m_pImageData->Copy(image);

		//每张图片保存的路径
		CString strSavePath(szSavePath);
		if (strSavePath.Right(1) == "\\")
		{
			strSavePath.Format(_T("%d%s"), nIndex, _T(".tif"));
		}
		else
		{
			strSavePath.Format(_T("\\%d%s"), nIndex, _T(".tif"));
		}

		//保存
		image.Save(strSavePath, CXIMAGE_FORMAT_TIF);
	}
}
BOOL Util::Image::ConvertType(ATL::CString strImagePathIn, ATL::CString  strImagePathOut, UINT32 bQuality/* = 50*/)
{
	//1.判断类型 
	ATL::CString strExtIn = FindExtension(strImagePathIn);

	INT32 nTypeIn = CxImage::GetTypeIdFromName(strExtIn);
	if (nTypeIn == CXIMAGE_FORMAT_UNKNOWN) {
		MessageBox(NULL, _T("输入的图片格式不对,请重新输入!"), _T("提示"), MB_OK);
		return FALSE;
	}

	ATL::CString strExtOut = FindExtension(strImagePathOut);
	INT32 nTypeOut = CxImage::GetTypeIdFromName(strExtOut);
	if (nTypeOut == CXIMAGE_FORMAT_UNKNOWN) {
		MessageBox(NULL, _T("输出的图片格式不对,请重新输入!"), _T("提示"), MB_OK);
		return FALSE;
	}

	//2对输入图片的大小分别做判断

	if (nTypeIn == CXIMAGE_FORMAT_TIF)
	{
	
	}

	//3图片转换
	CxImage image;

	image.Load(strImagePathIn.GetString(), nTypeIn);
	
	if (!image.IsValid()) 
	{
		MessageBox(NULL, _T("转换失败"), _T("提示"), MB_OK);
		return FALSE;
	}
	
	image.SetJpegQuality(bQuality);
	image.Save(strImagePathOut.GetString(), nTypeOut);
	return TRUE;
}



UINT8* Util::Image::SerializeBitmap(HBITMAP hbmp, UINT64 *bmpSizeOut)
{
	BITMAP bmpInfo;
	GetObject(hbmp, sizeof(BITMAP), &bmpInfo);
	LONG dx = bmpInfo.bmWidth;
	LONG dy = bmpInfo.bmHeight;

	DWORD bmpHeaderLen = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFO);
	DWORD bmpBytes = ((dx * 3 + 3) / 4) * 4 * dy + bmpHeaderLen;

	UINT8 *bmpData = new UINT8[bmpBytes];
	if (!bmpData)
		return nullptr;

	BITMAPINFO *bmi = (BITMAPINFO *)(bmpData + sizeof(BITMAPFILEHEADER));
	bmi->bmiHeader.biSize = sizeof(bmi->bmiHeader);
	bmi->bmiHeader.biWidth = dx;
	bmi->bmiHeader.biHeight = -dy;
	bmi->bmiHeader.biPlanes = 1;
	bmi->bmiHeader.biBitCount = 24;
	bmi->bmiHeader.biCompression = BI_RGB;

	HDC hDC = GetDC(nullptr);
	if (GetDIBits(hDC, hbmp, 0, dy, bmpData + bmpHeaderLen, bmi, DIB_RGB_COLORS)) {
		BITMAPFILEHEADER *bmpfh = (BITMAPFILEHEADER *)bmpData;  //增加文件头信息
		bmpfh->bfType = MAKEWORD('B', 'M');
		bmpfh->bfOffBits = bmpHeaderLen;
		bmpfh->bfSize = bmpBytes;
	}
	else {
		free(bmpData);
		bmpData = nullptr;
	}
	ReleaseDC(nullptr, hDC);

	if (bmpSizeOut)
		*bmpSizeOut = bmpBytes;
	return bmpData;
}


UINT8* Util::Image::AssembleBitmap(UINT8* bmpData,UINT32 uLen, INT32 width, INT32 height, UINT32 uBpc, UINT64* bmpSizeOut)
{
	//步幅
	int stride = ((width * 3 + 3) / 4) * 4;
	UINT8* dstImage = nullptr;
	if (uBpc == 4)
	{
		LONG lLineBytes24 = ((width * 24 + 31) / 32 * 4);  //对齐
		LONG lLineBytes4 = ((width * 4 + 7) / 8 * 1);		//对齐
		try
		{
			dstImage = new UINT8[lLineBytes24* height];
			if (nullptr == dstImage)
			{
				ATLASSERT(FALSE);
				return nullptr;
			}
		}
		catch (const std::bad_alloc& e)
		{
			ATLASSERT(FALSE);
			return nullptr;
		}
		
		INT32 n, j;
		for (INT32 i = 0; i < height; i++)
		{
			for (j = 0, n = 0; j < lLineBytes4; j++, n++)
			{
				if (lLineBytes4 *i + j > uLen)
				{
					ATLASSERT(FALSE);
					return nullptr;
				}
				UINT8 px = *(bmpData + lLineBytes4 /*width*/*i + j);

				UINT8 lo4, hi4;
				hi4 = (px & 0xf0) >> 4;
				lo4 =  px & 0x0f;
				hi4 |= 0xff & (hi4 << 4);
				lo4 |= 0xff & (lo4 << 4);

				*(dstImage + lLineBytes24*i + n) = lo4; //BGR
				n++;
				*(dstImage + lLineBytes24*i + n) = lo4;	//BGR
				n++;
				*(dstImage + lLineBytes24*i + n) = lo4;	//BGR
				n++;
				*(dstImage + lLineBytes24*i + n) = hi4;	//BGR
				n++;
				*(dstImage + lLineBytes24*i + n) = hi4;	//BGR
				n++;
				*(dstImage + lLineBytes24*i + n) = hi4;	//BGR
			}
		}

		bmpData = dstImage;

	}
	else if (uBpc == 8)
	{
		//1.流中图像的点阵数据是BGR的 组装位图时 必须把图转为RGB   24 位
		//RGB  To  BGR
		try
		{
			UINT8 *bmpDataOut = bmpData;
			bool is_grayscale = true;
			for (INT32 y = 0; y < height; y++)
			{
				if (y * stride > uLen)
				{
					ATLASSERT(FALSE);
					return nullptr;
				}
				UINT8 *in = bmpData + y * stride;
				UINT8 green = 0, blue = 0;
				for (INT32 x = 0; x < width; x++)
				{
					is_grayscale = is_grayscale && in[0] == in[1] && in[0] == in[2];
					blue = *in++;
					green = *in++;
					*bmpDataOut++ = *in++;
					*bmpDataOut++ = green;
					*bmpDataOut++ = blue;
				}
			}
		}
		catch (const std::bad_alloc& e)
		{
			ATLASSERT(FALSE);
			return nullptr;
		}

	}

#if 1

	//申请bitmap信息内存
	BITMAPINFO *bmpInfo = (BITMAPINFO *)calloc(1, sizeof(BITMAPINFOHEADER));
	if (!bmpInfo)
		return NULL;

	bmpInfo->bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);
	bmpInfo->bmiHeader.biWidth			= width;
	bmpInfo->bmiHeader.biHeight			= -height;
	bmpInfo->bmiHeader.biPlanes			= 1;
	bmpInfo->bmiHeader.biCompression	= BI_RGB;
	bmpInfo->bmiHeader.biBitCount		=  24;
	bmpInfo->bmiHeader.biSizeImage		= height * stride;
	bmpInfo->bmiHeader.biClrUsed		=  0;

	void *data = nullptr;
	HANDLE hMap = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, bmpInfo->bmiHeader.biSizeImage, nullptr);
	HBITMAP hbmp = CreateDIBSection(nullptr, bmpInfo, DIB_RGB_COLORS, &data, hMap, 0);
	if (hbmp){
		memcpy(data, bmpData, bmpInfo->bmiHeader.biSizeImage);
	}
	else{
		ATLASSERT(FALSE);
	}

	if (bmpInfo)
	{
		free(bmpInfo);
		bmpInfo = nullptr;
	}

	if (dstImage){
		delete[] dstImage;
		dstImage = nullptr;
	}
	//组装BMP文件头
	UINT32 uBmpHeadLen = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFO);
	UINT32 uBmpFileLen = stride * height + uBmpHeadLen;
	UINT8 *bmpBuffer = nullptr;
	try
	{
		bmpBuffer = new UINT8[uBmpFileLen];
		if (!bmpBuffer)
		{
			ATLASSERT(FALSE);
			return nullptr;
		}
	
	}
	catch (const std::bad_alloc& e)
	{
		ATLASSERT(FALSE);
		return nullptr;
	}

	BITMAPINFO *bmi = (BITMAPINFO *)(bmpBuffer + sizeof(BITMAPFILEHEADER));
	bmi->bmiHeader.biSize = sizeof(bmi->bmiHeader);
	bmi->bmiHeader.biWidth = width;
	bmi->bmiHeader.biHeight = -height;
	bmi->bmiHeader.biPlanes = 1;
	bmi->bmiHeader.biCompression = BI_RGB;
	bmi->bmiHeader.biBitCount =  24;
	bmi->bmiHeader.biSizeImage = height * stride;
	bmi->bmiHeader.biClrUsed = 0;

	HDC hDC = GetDC(nullptr);
	if (GetDIBits(hDC, hbmp, 0, height, bmpBuffer + uBmpHeadLen, bmi, DIB_RGB_COLORS)) {
		BITMAPFILEHEADER *bmpfh = (BITMAPFILEHEADER *)bmpBuffer;
		bmpfh->bfType    = MAKEWORD('B', 'M');
		bmpfh->bfOffBits = uBmpHeadLen;
		bmpfh->bfSize    = uBmpFileLen;
	}
	else {
		delete[] bmpBuffer;
		bmpBuffer = nullptr;
	}
	ReleaseDC(nullptr, hDC);
	::DeleteObject(hbmp);

#endif
	if (bmpSizeOut)
		*bmpSizeOut = uBmpFileLen;
	return bmpBuffer;
}

