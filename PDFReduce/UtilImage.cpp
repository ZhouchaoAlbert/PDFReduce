#include "UtilImage.h"
#include "ximage.h"


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

	//����ͼƬ,��image�ṹ��ֵ
	image.Load(strImagePath, CXIMAGE_FORMAT_TIF);

	//����һ��tiffͼƬ���ɶ�����ͼƬ�ϳɵ�
	nFrameCnt = image.GetNumFrames();

	//ÿ��ѭ��������1��tiffͼƬ�������浽m_pImageData��
	for (int nIndex = 0; nIndex < nFrameCnt; nIndex++)
	{
		image.SetFrame(nIndex);
		image.Load(strImagePath, CXIMAGE_FORMAT_TIF);

		//m_pImageData->Copy(image);

		//ÿ��ͼƬ�����·��
		CString strSavePath(szSavePath);
		if (strSavePath.Right(1) == "\\")
		{
			strSavePath.Format(_T("%d%s"), nIndex, _T(".tif"));
		}
		else
		{
			strSavePath.Format(_T("\\%d%s"), nIndex, _T(".tif"));
		}

		//����
		image.Save(strSavePath, CXIMAGE_FORMAT_TIF);
	}
}
BOOL Util::Image::ConvertType(ATL::CString strImagePathIn, ATL::CString  strImagePathOut, UINT32 bQuality/* = 50*/)
{
	//1.�ж����� 
	ATL::CString strExtIn = FindExtension(strImagePathIn);

	INT32 nTypeIn = CxImage::GetTypeIdFromName(strExtIn);
	if (nTypeIn == CXIMAGE_FORMAT_UNKNOWN) {
		MessageBox(NULL, _T("�����ͼƬ��ʽ����,����������!"), _T("��ʾ"), MB_OK);
		return FALSE;
	}

	ATL::CString strExtOut = FindExtension(strImagePathOut);
	INT32 nTypeOut = CxImage::GetTypeIdFromName(strExtOut);
	if (nTypeOut == CXIMAGE_FORMAT_UNKNOWN) {
		MessageBox(NULL, _T("�����ͼƬ��ʽ����,����������!"), _T("��ʾ"), MB_OK);
		return FALSE;
	}

	//2������ͼƬ�Ĵ�С�ֱ����ж�

	if (nTypeIn == CXIMAGE_FORMAT_TIF)
	{
	
	}

	//3ͼƬת��
	CxImage image;

	image.Load(strImagePathIn.GetString(), nTypeIn);
	
	if (!image.IsValid()) 
	{
		MessageBox(NULL, _T("ת��ʧ��"), _T("��ʾ"), MB_OK);
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
		BITMAPFILEHEADER *bmpfh = (BITMAPFILEHEADER *)bmpData;  //�����ļ�ͷ��Ϣ
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

UINT8* Util::Image::CreateRenderedBitmap(const char *bmpData, LONG dx, LONG dy, bool grayscale,UINT64 *bmpBytesOut)
{
	int stride = ((dx * (grayscale ? 1 : 3) + 3) / 4) * 4;

	BITMAPINFO *bmi = (BITMAPINFO *)calloc(1, sizeof(BITMAPINFOHEADER) + (grayscale ? 256 * sizeof(RGBQUAD) : 0));
	if (!bmi)
		return NULL;

	if (grayscale) {
		for (int i = 0; i < 256; i++) {
			bmi->bmiColors[i].rgbRed = bmi->bmiColors[i].rgbGreen = bmi->bmiColors[i].rgbBlue = (BYTE)i;
		}
	}

	bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi->bmiHeader.biWidth	= dx;
	bmi->bmiHeader.biHeight = -dy;
	bmi->bmiHeader.biPlanes = 1;
	bmi->bmiHeader.biCompression = BI_RGB;
	bmi->bmiHeader.biBitCount = grayscale ? 8 : 24;
	bmi->bmiHeader.biSizeImage = dy * stride;
	bmi->bmiHeader.biClrUsed = grayscale ? 256 : 0;

	void *data = nullptr;
	HANDLE hMap = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, bmi->bmiHeader.biSizeImage, nullptr);
	HBITMAP hbmp = CreateDIBSection(nullptr, bmi, DIB_RGB_COLORS, &data, hMap, 0);
	if (hbmp)
		memcpy(data, bmpData, bmi->bmiHeader.biSizeImage);
	free(bmi);
	UINT64 uBytesOut = 0;

	UINT8* bmpBuf = SerializeBitmap(hbmp, &uBytesOut);
	if (bmpBytesOut)
		*bmpBytesOut = uBytesOut;
	return bmpBuf;
}


UINT8* Util::Image::AssembleBitmap(UINT8* bmpData, INT32 width, INT32 height, BOOL bGrayScale, UINT64* bmpSizeOut)
{

	
	LONG lLineBytes24 = ((width * 24 + 31) / 32 * 4);
	LONG lLineBytes4 = ((width * 4 + 31) / 32 * 4);

	UINT8* dstImage = new UINT8[lLineBytes24* height];
	int n, j;
	for (int i = 0; i < height; i++)
	{
		for (j = 0, n = 0; j < lLineBytes4; j++, n++)
		{
			BYTE gray = *(bmpData + lLineBytes4 /*width*/*i + j);

			UINT8 lo4, hi4;
			hi4   = (gray & 0xf0) >> 4;
			lo4   = gray & 0x0f;

			*(dstImage + lLineBytes24*i + n) = lo4;  //BGR
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

	//����
	int stride = ((width * (bGrayScale ? 1 : 3) + 3) / 4) * 4; 
	//1.����ͼ��ĵ���������BGR�� ��װλͼʱ �����ͼתΪRGB
	//RGB  To  BGR
// 	UINT8 *bmpDataOut = bmpData;
// 	bool is_grayscale = true;
// 	for (int y = 0; y < height; y++) 
// 	{
// 		UINT8 *in = bmpData + y * stride; 
// 		UINT8 green = 0, blue = 0;
// 		for (int x = 0; x < width; x++) 
// 		{
// 			is_grayscale = is_grayscale && in[0] == in[1] && in[0] == in[2];
// 			blue			= *in++;
// 			green			= *in++;
// 			*bmpDataOut++	= *in++;
// 			*bmpDataOut++	= green;
// 			*bmpDataOut++	= blue;
// 		}
// 	}

	//����bitmap��Ϣ�ڴ�
	BITMAPINFO *bmpInfo = (BITMAPINFO *)calloc(1, sizeof(BITMAPINFOHEADER) + (bGrayScale ? 256 * sizeof(RGBQUAD) : 0));
	if (!bmpInfo)
		return NULL;

	if (bGrayScale){
		for (INT32 nIndex = 0; nIndex < 256; nIndex++) {
			bmpInfo->bmiColors[nIndex].rgbRed = bmpInfo->bmiColors[nIndex].rgbGreen = bmpInfo->bmiColors[nIndex].rgbBlue = (BYTE)nIndex;
		}
	}

	bmpInfo->bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);
	bmpInfo->bmiHeader.biWidth			= width;
	bmpInfo->bmiHeader.biHeight			= -height;
	bmpInfo->bmiHeader.biPlanes			= 1;
	bmpInfo->bmiHeader.biCompression	= BI_RGB;
	bmpInfo->bmiHeader.biBitCount		= bGrayScale ? 8 : 24;
	bmpInfo->bmiHeader.biSizeImage		= height * stride;
	bmpInfo->bmiHeader.biClrUsed		= bGrayScale ? 256 : 0;

	void *data = nullptr;
	HANDLE hMap = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, bmpInfo->bmiHeader.biSizeImage, nullptr);
	HBITMAP hbmp = CreateDIBSection(nullptr, bmpInfo, DIB_RGB_COLORS, &data, hMap, 0);
	if (hbmp)
		memcpy(data, bmpData, bmpInfo->bmiHeader.biSizeImage);
	free(bmpInfo);
  
	//��װBMP�ļ�ͷ
	UINT32 uBmpHeadLen = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFO);
	UINT32 uBmpFileLen = stride * height + uBmpHeadLen;

	UINT8 *bmpBuffer = new UINT8[uBmpFileLen];
	if (!bmpBuffer)
		return nullptr;

	BITMAPINFO *bmi = (BITMAPINFO *)(bmpBuffer + sizeof(BITMAPFILEHEADER));
	bmi->bmiHeader.biSize = sizeof(bmi->bmiHeader);
	bmi->bmiHeader.biWidth = width;
	bmi->bmiHeader.biHeight = -height;
	bmi->bmiHeader.biPlanes = 1;
	bmi->bmiHeader.biCompression = BI_RGB;
	bmi->bmiHeader.biBitCount = bGrayScale ? 8 : 24;
	bmi->bmiHeader.biSizeImage = height * stride;
	bmi->bmiHeader.biClrUsed = bGrayScale ? 256 : 0;

	HDC hDC = GetDC(nullptr);
	if (GetDIBits(hDC, hbmp, 0, height, bmpBuffer + uBmpHeadLen, bmi, DIB_RGB_COLORS)) {
		BITMAPFILEHEADER *bmpfh = (BITMAPFILEHEADER *)bmpBuffer;
		bmpfh->bfType    = MAKEWORD('B', 'M');
		bmpfh->bfOffBits = uBmpHeadLen;
		bmpfh->bfSize    = uBmpFileLen;
	}
	else {
		free(bmpBuffer);
		bmpBuffer = nullptr;
	}
	ReleaseDC(nullptr, hDC);

	if (bmpSizeOut)
		*bmpSizeOut = uBmpFileLen;
	return bmpBuffer;
}

/*
CxImage image(bmpBuf, bmpBytesOut, CXIMAGE_FORMAT_BMP);
if (image.IsValid())
{
image.SetJpegQuality(50);
image.Save(_T("E:\\test\\convert\\bitmap.jpg"), CXIMAGE_FORMAT_JPG);
}
*/