#include "PdfCompress.h"
#include <sstream>  
#include "zlib.h"
#include <io.h>
#include <fstream>
#include<iostream>
#include <Windows.h>
#include "UtilImage.h"



fz_context_s*  CPdfCompress::ctx = NULL;
pdf_document* CPdfCompress::doc = NULL;

CPdfCompress::CPdfCompress()
{
}

CPdfCompress::~CPdfCompress()
{
}


UINT32 CPdfCompress::StartThread(ATL::CString strPdfPath, ATL::CString strPassword, ATL::CString strPdfOutPath)
{
	m_strPdfPath	= strPdfPath;
	m_strPassword	= strPassword;
	m_strPdfOutPath = strPdfOutPath;

	UINT32 uiThreadID = 0;
	HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadProc, (void*)this, 0, &uiThreadID);
	if (hThread == NULL)
		return 0;
	return uiThreadID;
}

// �̻߳ص�
UINT WINAPI  CPdfCompress::ThreadProc(void* pVoid)
{
	CPdfCompress *pThreadCallback = static_cast<CPdfCompress *>(pVoid);
	if (pThreadCallback)
	{
		pThreadCallback->Run();
	}
	return 0;
}


void CPdfCompress::Run()
{
	if (Init(m_strPdfPath, m_strPassword))
	{
		//test
		PraseResImage("E:\\test\\convert");
	}
}



//��ʼ������
BOOL CPdfCompress::Init(ATL::CString strPdfPath, ATL::CString strPassword)
{
	if (strPdfPath.IsEmpty())
	{
		ATLASSERT(FALSE);
		return FALSE;
	}
	ATL::CW2A szPdfPath(strPdfPath.GetString(), CP_UTF8);
	ATL::CW2A szPassword(strPassword.GetString(), CP_UTF8);
	//���������� ����ȫ����Ϣ
	ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
	if (!ctx)
	{
		ATLASSERT(FALSE);
		return FALSE;
	}
	//���ĵ�
	doc = pdf_open_document_no_run(ctx, szPdfPath.m_psz);
	if (!doc)
	{
		ATLASSERT(FALSE);
		return FALSE;
	}
	//PDF�Ƿ���Ҫ���뼰�����Ȩ
	if (pdf_needs_password(doc) && !pdf_authenticate_password(doc, szPassword.m_psz))
	{
		ATLASSERT(FALSE);
		return FALSE;
	}
	return TRUE;
}

//��ȡPDFҳ
UINT32 CPdfCompress::GetPdfPages(pdf_document* doc)
{
	return pdf_count_pages(doc);
}

//�Ƿ���ͼƬ
BOOL CPdfCompress::IsImage(pdf_obj* obj)
{
	pdf_obj *type = pdf_dict_gets(obj, "Subtype");
	return pdf_is_name(type) && !strcmp(pdf_to_name(type), "Image");
}


BOOL CPdfCompress::IsRGBColorSpace(pdf_obj* obj)
{
	pdf_obj *type = pdf_dict_gets(obj, "ColorSpace");
	return pdf_is_name(type) && !strcmp(pdf_to_name(type), "DeviceRGB");
}

BOOL CPdfCompress::IsFlateDecode(pdf_obj* obj)
{
	pdf_obj *type = pdf_dict_gets(obj, "Filter");
	return pdf_is_name(type) && !strcmp(pdf_to_name(type), "FlateDecode");
}

BOOL  CPdfCompress::WritePixmap(fz_context *ctx, fz_pixmap *pix, std::string filepath, int rgb)
{
	BOOL bRet = FALSE;

	fz_pixmap *converted = NULL;
	if (!pix)
	{
		return bRet;
	}
	if (rgb && pix->colorspace && pix->colorspace != fz_device_rgb(ctx))
	{
		fz_irect bbox;
		converted = fz_new_pixmap_with_bbox(ctx, fz_device_rgb(ctx), fz_pixmap_bbox(ctx, pix, &bbox));
		fz_convert_pixmap(ctx, converted, pix);
		pix = converted;
	}
	if (pix->n <= 4)
	{
		fz_write_png(ctx, pix, (char*)filepath.c_str(), 0);
		bRet = TRUE;
	}
	else
	{
		ATLASSERT(FALSE);
	}
	fz_drop_pixmap(ctx, converted);
	return bRet;
}

BOOL CPdfCompress::SaveImage(std::string szSavePath, INT32 nNum)
{
	BOOL bRet = FALSE;
	if (szSavePath.empty())
	{
		ATLASSERT(FALSE);
		return bRet;
	}
	fz_image   *image;
	fz_pixmap  *pix;
	pdf_obj    *ref;


	ref = pdf_new_indirect(doc, nNum, 0);

	//����ͼƬ���ļ�
	image = pdf_load_image(doc, ref);
	pix = fz_new_pixmap_from_image(ctx, image, 0, 0);
	fz_drop_image(ctx, image);

	//rgb mode
	bRet = WritePixmap(ctx, pix, szSavePath, 1);

	fz_drop_pixmap(ctx, pix);
	pdf_drop_obj(ref);

	return bRet;
}



//������Դ��Ϣ
void CPdfCompress::PraseResImage(std::string szSavePath)
{
	UINT32 uObjNums = pdf_count_objects(doc);
	for (UINT32 uIndex = 1; uIndex < uObjNums; uIndex++)
	{
		pdf_obj  *obj;
		obj = pdf_load_object(doc, uIndex, 0);
		if (IsImage(obj) /*&& IsRGBColorSpace(obj)*/ /*&& IsFlateDecode(obj)*/)
		{

			std::stringstream os;
			os << szSavePath << "\\img-" << uIndex << ".png";
			if (SaveImage(os.str(), uIndex))
			{

				if (ImageConvert(szSavePath, uIndex))
					WriteDataToStream(obj, szSavePath, uIndex);
#if 0
				DeleteFileA(os.str().c_str());
				std::stringstream os2;
				os2 << szSavePath << "\\img-" << uIndex << ".jpg";
				DeleteFileA(os2.str().c_str());
#endif
			}

		}
		pdf_drop_obj(obj);
	}
	std::stringstream os;
	os << szSavePath << "\\out.pdf";
	std::string strOutPath = os.str();
	pdf_write_document(doc, (char*)strOutPath.c_str(), nullptr);


	//
	if (doc)
	{
		pdf_close_document(doc);
		doc = NULL;
		fz_free_context(ctx);
	}

}

BOOL CPdfCompress::ImageConvert(std::string szSavePath, INT32 nNum)
{
	BOOL bRet = FALSE;
	if (szSavePath.empty())
	{
		ATLASSERT(FALSE);
		return bRet;
	}

	std::stringstream osin;
	osin << szSavePath << "\\img-" << nNum << ".png";

	std::stringstream osout;
	osout << szSavePath << "\\img-" << nNum << ".jpg";

	ATL::CA2W strSrcImagePath(osin.str().c_str(), CP_ACP);
	ATL::CA2W strImageOutPath(osout.str().c_str(), CP_ACP);

	bRet = Util::Image::ConvertType(strSrcImagePath, strImageOutPath);

	return bRet;
}





BOOL  CPdfCompress::WriteDataToStream(pdf_obj* obj, std::string szSavePath, INT32 nNum)
{
	//ͼƬ·��
	std::stringstream osout;
	osout << szSavePath << "\\img-" << nNum << ".jpg";

	std::ifstream fin(osout.str().c_str(), std::ios::binary);
	fin.seekg(0, std::ios::end);
	UINT32 uSize = fin.tellg();
	UINT8* szBuf = new (std::nothrow)UINT8[uSize + 1];
	memset(szBuf, 0, sizeof(szBuf));
	fin.seekg(0, std::ios::beg);
	fin.read((char*)szBuf, sizeof(char) * uSize);
	fin.close();
#if 0
	UINT32 uDestLen = uSize;
	UINT8* buffer_dest = new (std::nothrow)UINT8[uSize];
	compress(buffer_dest, (uLong*)&uDestLen, szBuf, uSize);
#endif

	fz_buffer * stm_buf = fz_new_buffer(ctx, (int)uSize);
	memcpy(stm_buf->data, szBuf, uSize);
	stm_buf->len = uSize;

#if 0
	CStringA strTest;
	for (UINT32 i = 0; i < uSize; i++)
	{
		strTest.AppendFormat("%02X", szBuf[i]);
	}
	FILE* fp;
	fp = fopen("c1111.txt", "wb");
	fwrite(strTest, strTest.GetLength(), 1, fp);
	fclose(fp);
#endif

	//���³���
	pdf_obj *type = pdf_dict_gets(obj, "Length");
	if (pdf_is_indirect(type))
	{
		int num = pdf_to_num(type);
		int gen = pdf_to_gen(type);
		pdf_obj* lenobj = pdf_load_object(doc, num, gen);
		pdf_set_int(lenobj, uSize);
		pdf_update_object(doc, num, lenobj);
	}
	else if (pdf_is_int(obj))
	{
		pdf_set_int(type, uSize);
	}
	pdf_dict_dels(obj, "Filter");
	pdf_dict_puts_drop(obj, "Filter", pdf_new_name(doc, "DCTDecode"));

	//������
	pdf_update_stream(doc, nNum, stm_buf);

	//delete[] buffer_dest;
	delete[]  szBuf;

	return TRUE;
}

