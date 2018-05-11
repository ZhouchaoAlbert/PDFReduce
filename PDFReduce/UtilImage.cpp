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
