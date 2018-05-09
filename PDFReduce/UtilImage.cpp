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
BOOL Util::Image::ConvertType(LPCTSTR szImagePathIn, LPCTSTR szImagePathOut, UINT32 bQuality/* = 50*/)
{
	//1.�ж����� 
	TCHAR* szExtIn = FindExtension(szImagePathIn);
	_tcslwr(szExtIn);
	INT32 nTypeIn = CxImage::GetTypeIdFromName(szExtIn);
	if (nTypeIn == CXIMAGE_FORMAT_UNKNOWN) {
		MessageBox(NULL, _T("�����ͼƬ��ʽ����,����������!"), _T("��ʾ"), MB_OK);
		return FALSE;
	}

	TCHAR* szExtOut = FindExtension(szImagePathOut);
	_tcslwr(szExtOut);
	INT32 nTypeOut = CxImage::GetTypeIdFromName(szExtOut);
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

	image.Load((const TCHAR*)szImagePathIn, nTypeIn);
	
	if (!image.IsValid()) 
	{
		MessageBox(NULL, _T("ת��ʧ��"), _T("��ʾ"), MB_OK);
		return FALSE;
	}
	
	image.SetJpegQuality(bQuality);
	image.Save((const TCHAR*)szImagePathOut, nTypeOut);
	return TRUE;
}
