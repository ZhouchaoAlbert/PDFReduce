#include "PdfCompress.h"
#include <sstream>  

#include <io.h>
#include <fstream>
#include<iostream>
#include <Windows.h>
#include "UtilImage.h"
#include "UtilPath.h"

#include <vector>

fz_context_s*  CPdfCompress::ctx = NULL;
pdf_document* CPdfCompress::doc = NULL;

CPdfCompress::CPdfCompress()
{
	
}

CPdfCompress::~CPdfCompress()
{
}


UINT32 CPdfCompress::StartThread(ATL::CString strPdfPath, ATL::CString strPassword, ATL::CString strPdfOutPath, HWND hWnd)
{
	Util::Path::GetImageTempPath(TRUE);
	m_strPdfPath	= strPdfPath;
	m_strPassword	= strPassword;
	m_strPdfOutPath = strPdfOutPath;
	m_hWnd			= hWnd;
// 	if (Init(m_strPdfPath, m_strPassword))
// 	{
// 
// 		pdf_obj* root = pdf_dict_gets(pdf_trailer(doc), "Root");
 //		pdf_obj* obj = pdf_dict_gets(root, "Outlines");
// 
// 		if (pdf_is_indirect(obj))
// 		{
// 			int num = pdf_to_num(obj);
// 			int gen = pdf_to_gen(obj);
// 			pdf_delete_object(doc, num);
// // 			pdf_obj* sub_obj = pdf_load_object(doc, num, gen);
// // 			pdf_drop_obj(sub_obj);
// // 			sub_obj = NULL;
// 		}
// 		pdf_obj* oa_obj = pdf_dict_gets(root, "OpenAction");
// 		if (pdf_is_indirect(oa_obj))
// 		{
// 			int num = pdf_to_num(oa_obj);
// 			int gen = pdf_to_gen(oa_obj);
// 			pdf_delete_object(doc, num);
// // 			pdf_obj* sub_obj2 = pdf_load_object(doc, num, gen);
// // 			pdf_drop_obj(sub_obj2);
// // 			sub_obj2 = NULL;
// 		}
// 		pdf_dict_dels(root, "Outlines");
// 		pdf_dict_dels(root, "OpenAction");
// 		pdf_dict_dels(root, "PageMode");
// 		
// 
// 		ATL::CW2A szPdfOutPath(m_strPdfOutPath, CP_UTF8);
// 		pdf_write_document(doc, (char*)szPdfOutPath, nullptr);
// 		if (doc)
// 		{
// 			pdf_close_document(doc);
// 			doc = NULL;
// 			fz_free_context(ctx);
// 			ctx = NULL;
// 		}
// 		//PraseResImage();
// 	}
// 	return 0;

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
		PraseResImage();
	}
}

void CPdfCompress::JumpThreadSetProcess(INT32 i, INT32 j)
{
	::PostMessage(m_hWnd, WM_UI_PROCESS, (WPARAM)i, (LPARAM)j);
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

BOOL  CPdfCompress::WritePixmap(fz_context *ctx, fz_pixmap *pix, ATL::CString strImagePath, int rgb)
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
		ATL::CW2A szImagePath(strImagePath, CP_UTF8);
		fz_write_png(ctx, pix, (char*)szImagePath, 0);
		bRet = TRUE;
	}
	else
	{
		ATLASSERT(FALSE);
	}
	fz_drop_pixmap(ctx, converted);
	return bRet;
}

BOOL CPdfCompress::SaveImage(ATL::CString strImagePath, INT32 nNum)
{
	BOOL bRet = FALSE;
	if (strImagePath.IsEmpty())
	{
		ATLASSERT(FALSE);
		return bRet;
	}

	fz_image   *image = NULL;
	fz_pixmap  *pix = NULL;
	pdf_obj    *ref = NULL;


	ref = pdf_new_indirect(doc, nNum, 0);
	UINT32 u = 0;
	//保存图片到文件
	image = pdf_load_image(doc, ref);
	if (!image)
	{
		return bRet;
	}
	//std::string strBuffer((const char*)image->buffer->buffer->data,image->buffer->buffer->len);
	
	fz_try(ctx)
	{
		pix = fz_new_pixmap_from_image(ctx, image, 0, 0);
		if (!pix)
		{
			return bRet;
		}

	}
	fz_catch(ctx)
	{
		u = GetLastError();
	}

	fz_drop_image(ctx, image);
	//rgb mode
	bRet = WritePixmap(ctx, pix, strImagePath, 1);

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
	//Test 数据写本地
	if (FZ_IMAGE_JPEG  == fz_com_buffer->params.type)
	{
		CString strSrcImage;
		strSrcImage.Format(_T("%s\\yueshu-img-%d.jpg"), Util::Path::GetImageTempPath(), num);
		CString strDestImage;
		strDestImage.Format(_T("%s\\sec-yueshu-img-%d.jpg"), Util::Path::GetImageTempPath(), num);
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
				WriteDataToStream(obj, strDestImage, num);
			}

		}
		DeleteFile(strSrcImage);
		DeleteFile(strDestImage);
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
		//1.对buffer 数据解码
		//2.位图+像素


		return TRUE;
	}
	else if (FZ_IMAGE_LZW == fz_com_buffer->params.type)
	{
		return TRUE;
	}
	return FALSE;
}
//解析资源信息
void CPdfCompress::PraseResImage()
{
	std::vector<UINT32> vecNum;
	UINT32 uObjNums = pdf_count_objects(doc);
	for (UINT32 uIndex = 1; uIndex < uObjNums; uIndex++)
	{
		pdf_obj  *obj;
		obj = pdf_load_object(doc, uIndex, 0);
		if (IsImage(obj) /*&& IsRGBColorSpace(obj)*/ /*&& IsFlateDecode(obj)*/)
		{
			vecNum.push_back(uIndex);
		}
		pdf_drop_obj(obj);
	}
	static UINT32 i = 1;

	for (std::vector<UINT32>::iterator it = vecNum.begin(); it != vecNum.end();it++)
	{
		
		//获取流数据
		if (IsCompressImageStream(doc, *it, 0))
		{
#if 1
			CString strSrcImage, strDestImage;
			strSrcImage.Format(_T("%s\\yueshu-img-%d.png"), Util::Path::GetImageTempPath(), *it);
			strDestImage.Format(_T("%s\\yueshu-img-%d.jpg"), Util::Path::GetImageTempPath(), *it);

			if (SaveImage(strSrcImage, *it))
			{
				if (ImageConvert(strSrcImage, strDestImage, *it) && ImageSizeCompare(strSrcImage,strDestImage))
				{
					//获取文件大小  
					pdf_obj  *obj;
					obj = pdf_load_object(doc, *it, 0);
					WriteDataToStream(obj, strDestImage, *it);
					pdf_drop_obj(obj);
				}
			}
			DeleteFile(strSrcImage);
			DeleteFile(strDestImage);
#endif
		}
		JumpThreadSetProcess(i,vecNum.size());
		i++;
	}
	i = 1;
	ATL::CW2A szPdfOutPath(m_strPdfOutPath, CP_UTF8);	
	fz_write_options opts;
	opts.do_garbage =2;
	pdf_write_document(doc, (char*)szPdfOutPath, &opts);
	//
	if (doc)
	{
		pdf_close_document(doc);
		doc = NULL;
		fz_free_context(ctx);
		ctx = NULL;
	}
}

BOOL CPdfCompress::ImageConvert(ATL::CString strSrcImagePath, ATL::CString strDestImagePath, INT32 nNum)
{
	BOOL bRet = FALSE;
	if (strSrcImagePath.IsEmpty() || strDestImagePath.IsEmpty())
	{
		ATLASSERT(FALSE);
		return bRet;
	}
	ATL::CString strNum;
	strNum.Format(_T("yueshu-img-%d."), nNum);
	if (strSrcImagePath.Find(strNum) <  0 || strDestImagePath.Find(strNum) < 0)
	{
		return bRet;
	}
	bRet = Util::Image::ConvertType(strSrcImagePath, strDestImagePath);
	return bRet;
}


BOOL  CPdfCompress::WriteDataToStream(pdf_obj* obj, ATL::CString  strDestImagePath, INT32 nNum)
{
	ATL::CW2A szDestImagePath(strDestImagePath, CP_UTF8);

	std::ifstream fin(szDestImagePath, std::ios::binary);
	fin.seekg(0, std::ios::end);
	UINT32 uSize = fin.tellg();
	UINT8* szBuf = new (std::nothrow)UINT8[uSize];
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
	pdf_dict_dels(obj, "ColorSpace");
	pdf_dict_dels(obj, "DecodeParms");
	pdf_dict_puts_drop(obj, "ColorSpace", pdf_new_name(doc, "DeviceRGB"));
	pdf_dict_puts_drop(obj, "BitsPerComponent", pdf_new_int(doc,8));

	//更新流
	pdf_update_stream(doc, nNum, stm_buf);
	fz_drop_buffer(ctx, stm_buf);
	//delete[] buffer_dest;
	delete[]  szBuf;

	return TRUE;
}

BOOL CPdfCompress::ImageSizeCompare(CString strSrcIamge, CString strDestImage)
{
	HANDLE h1 = CreateFile(strSrcIamge, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (h1 == INVALID_HANDLE_VALUE)
		return FALSE;
	LARGE_INTEGER size1;
	BOOL ok = GetFileSizeEx(h1, &size1);
	if (!ok)
		return FALSE;
	UINT32 uSrcSize = size1.QuadPart;

	HANDLE h2 = CreateFile(strDestImage, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (h2 == INVALID_HANDLE_VALUE)
		return FALSE;
	LARGE_INTEGER size2;
	BOOL ok2 = GetFileSizeEx(h2, &size2);
	if (!ok2)
		return FALSE;
	UINT32 uDestSize = size2.QuadPart;

	if (uDestSize > uSrcSize)
	{
		return FALSE;
	}
	return TRUE;
}