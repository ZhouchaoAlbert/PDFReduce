#include "PdfCompress.h"
#include <sstream>  

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

// 线程回调
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



//初始化操作
BOOL CPdfCompress::Init(ATL::CString strPdfPath, ATL::CString strPassword)
{
	if (strPdfPath.IsEmpty())
	{
		ATLASSERT(FALSE);
		return FALSE;
	}
	ATL::CW2A szPdfPath(strPdfPath.GetString(), CP_UTF8);
	ATL::CW2A szPassword(strPassword.GetString(), CP_UTF8);
	//创建上下文 包含全局信息
	ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
	if (!ctx)
	{
		ATLASSERT(FALSE);
		return FALSE;
	}
	//打开文档
	doc = pdf_open_document_no_run(ctx, szPdfPath.m_psz);
	if (!doc)
	{
		ATLASSERT(FALSE);
		return FALSE;
	}
	//PDF是否需要密码及密码鉴权
	if (pdf_needs_password(doc) && !pdf_authenticate_password(doc, szPassword.m_psz))
	{
		ATLASSERT(FALSE);
		return FALSE;
	}
	return TRUE;
}

//获取PDF页
UINT32 CPdfCompress::GetPdfPages(pdf_document* doc)
{
	return pdf_count_pages(doc);
}

//是否是图片
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

	//保存图片到文件
	image = pdf_load_image(doc, ref);


	pix = fz_new_pixmap_from_image(ctx, image, 0, 0);
	fz_drop_image(ctx, image);
	//rgb mode
	bRet = WritePixmap(ctx, pix, szSavePath, 1);

	fz_drop_pixmap(ctx, pix);
	pdf_drop_obj(ref);

	return bRet;
}

BOOL CPdfCompress::IsCompressImageStream(pdf_document* doc, INT32 num, INT32 gen)
{
	fz_compressed_buffer * fz_com_buffer = pdf_load_compressed_stream(doc, num, gen);
	if (!fz_com_buffer)
	{
		ATLASSERT(FALSE);
		return FALSE;
	}
	std::string strBuf((char*)fz_com_buffer->buffer->data, fz_com_buffer->buffer->len);
	//Test 数据写本地
	if (FZ_IMAGE_JPEG  == fz_com_buffer->params.type)
	{
		CString strImageFolder = _T("E:\\test\\convert");
		CString strSrcImage;
		strSrcImage.Format(_T("%s\\yueshu-img-%d.jpg"), strImageFolder, num);
		CString strDestImage;
		strDestImage.Format(_T("%s\\sec-yueshu-img-%d.jpg"), strImageFolder, num);
		FILE* fp = NULL;
		fp = _wfopen(strSrcImage, _T("wb"));
		if (NULL != fp)
		{
			fwrite((char*)fz_com_buffer->buffer->data, fz_com_buffer->buffer->len, 1, fp);
			fclose(fp);
			
			if (Util::Image::ConvertType(strSrcImage, strDestImage))
			{
				pdf_obj  *obj;
				obj = pdf_load_object(doc, num, 0);
				if (WriteDataToStream(obj, strDestImage, num))
				{

				}
			}
		}
		return FALSE;
	}
	else if (FZ_IMAGE_FAX == fz_com_buffer->params.type)
	{
		return TRUE;
	}
	else if (FZ_IMAGE_RLD == fz_com_buffer->params.type)
	{
		return TRUE;
	}
	else if (FZ_IMAGE_FLATE == fz_com_buffer->params.type)
	{
		return TRUE;
	}
	else if (FZ_IMAGE_LZW == fz_com_buffer->params.type)
	{
		return TRUE;
	}
	return FALSE;
}
//解析资源信息
void CPdfCompress::PraseResImage(std::string szSavePath)
{
	UINT32 uObjNums = pdf_count_objects(doc);
	for (UINT32 uIndex = 1; uIndex < uObjNums; uIndex++)
	{
		pdf_obj  *obj;
		obj = pdf_load_object(doc, uIndex, 0);
		if (IsImage(obj) /*&& IsRGBColorSpace(obj)*/ /*&& IsFlateDecode(obj)*/)
		{
			//获取流数据
			if (IsCompressImageStream(doc, uIndex, 0))
			{
				std::stringstream os;
				os << szSavePath << "\\img-" << uIndex << ".png";
				if (SaveImage(os.str(), uIndex))
				{
					CString strImageFolder = _T("E:\\test\\convert");
					CString strSrcImage;
					strSrcImage.Format(_T("%s\\yueshu-%d.png"), strImageFolder,uIndex);
					CString strDestImage;
					strDestImage.Format(_T("%s\\yueshu-%d.jpg"), strImageFolder,uIndex);

					if (ImageConvert(strSrcImage, strDestImage, uIndex))
					{
						WriteDataToStream(obj, strDestImage, uIndex);
					}
#if 0
					DeleteFileA(os.str().c_str());
					std::stringstream os2;
					os2 << szSavePath << "\\img-" << uIndex << ".jpg";
					DeleteFileA(os2.str().c_str());
#endif
				}
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

BOOL CPdfCompress::ImageConvert(CString strSrcImagePath, CString strDestImagePath, INT32 nNum)
{
	BOOL bRet = FALSE;
	if (strSrcImagePath.IsEmpty() || strDestImagePath.IsEmpty())
	{
		ATLASSERT(FALSE);
		return bRet;
	}
	CString strNum;
	strNum.Format(_T("yueshu-img-%d."), nNum);
	if (strSrcImagePath.Find(strNum) <  0 || strSrcImagePath.Find(strNum))
	{
		return bRet;
	}
	bRet = Util::Image::ConvertType(strSrcImagePath, strDestImagePath);
	return bRet;
}




BOOL  CPdfCompress::WriteDataToStream(pdf_obj* obj, CString  strDestImagePath, INT32 nNum)
{
	ATL::CW2A szDestImagePath(strDestImagePath, CP_UTF8);

	std::ifstream fin(szDestImagePath, std::ios::binary);
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

	//更新长度
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

	//更新流
	pdf_update_stream(doc, nNum, stm_buf);

	//delete[] buffer_dest;
	delete[]  szBuf;

	return TRUE;
}

