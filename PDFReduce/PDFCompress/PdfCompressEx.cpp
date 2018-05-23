#include "PdfCompressEx.h"
#include <sstream>  
#include <io.h>
#include <fstream>
#include<iostream>
#include <Windows.h>

#include <vector>
#include "ximage.h"

#include"..\Help\UtilPath.h"
#include "..\Help\UtilImage.h"

#define  WM_MYCOMPRESS_UI_TASK WM_USER + 1001

CPdfCompressEx::CPdfCompressEx() :
m_bRun(FALSE),
m_ctx(NULL),
m_doc(NULL)
{
	AddMsg(WM_MYCOMPRESS_UI_TASK);
	Start();
}

CPdfCompressEx::~CPdfCompressEx()
{
	Stop();
}

void CPdfCompressEx::AddTask(ST_PDFINFO_NODE TaskNode)
{
	m_CritSec.Lock();
	m_TaskList.push_back(TaskNode);
	m_CritSec.Unlock();	
}

UINT32 CPdfCompressEx::StartCompress()
{
	if (m_bRun)
	{
		ATLASSERT(FALSE);
		return 0;
	}
	if (!Util::Path::IsFileExist(Util::Path::GetImageTempPath(FALSE)))
	{
		Util::Path::GetImageTempPath(TRUE);
	}

	Thread::IThreadManager* pThreadManager = NULL;
	Thread::GetBaseThreadObject((void**)&pThreadManager);
	if (NULL == pThreadManager)
	{
		ATLASSERT(FALSE);
		return 0;
	}
	m_bRun = TRUE;
	bool bRet = pThreadManager->CreateThread(this, m_uThreadId);
	if (!bRet) m_bRun = FALSE;
	return m_uThreadId;
}

void CPdfCompressEx::SetProcessCallback(std::function<void(INT32 nCode, INT32 nId, INT32 nVal, CString strOutInfo)>Func_CallBack)
{
	m_Func_CallBack = Func_CallBack;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//初始化操作
BOOL CPdfCompressEx::DocumentInit(ATL::CString strPdfPath, ATL::CString strPassword)
{
	if (m_doc){
		pdf_close_document(m_doc);
	}
	if (m_ctx){
		fz_free_context(m_ctx);	
	}
	m_doc = nullptr;
	m_ctx = nullptr;

	if (strPdfPath.IsEmpty())
	{
		ATLASSERT(FALSE);
		return FALSE;
	}
	ATL::CW2A szPdfPath(strPdfPath.GetString(), CP_UTF8);
	ATL::CW2A szPassword(strPassword.GetString(), CP_UTF8);
	//创建上下文 包含全局信息
	m_ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
	if (!m_ctx)
	{
		ATLASSERT(FALSE);
		return FALSE;
	}
	//打开文档
	m_doc = pdf_open_document_no_run(m_ctx, szPdfPath.m_psz);
	if (!m_doc)
	{
		ATLASSERT(FALSE);
		return FALSE;
	}
	//PDF是否需要密码及密码鉴权
	if (pdf_needs_password(m_doc) && !pdf_authenticate_password(m_doc, szPassword.m_psz))
	{
		ATLASSERT(FALSE);
		return FALSE;
	}
	return TRUE;
}

//获取PDF页
UINT32 CPdfCompressEx::GetPdfPages(pdf_document* doc)
{
	return pdf_count_pages(doc);
}

//是否是图片
BOOL CPdfCompressEx::IsImageType(pdf_obj* dict)
{
	pdf_obj *type = pdf_dict_gets(dict, "Subtype");
	return pdf_is_name(type) && !strcmp(pdf_to_name(type), "Image");
}

//彩色空间是 RGB
BOOL CPdfCompressEx::IsRGBColorSpace(pdf_obj* dict)
{
	pdf_obj *colorspace = pdf_dict_gets(dict, "ColorSpace");
	return pdf_is_name(colorspace) && !strcmp(pdf_to_name(colorspace), "DeviceRGB");
}

//zlib压缩
BOOL CPdfCompressEx::IsFlateDecode(pdf_obj* dict)
{
	pdf_obj *filter;
	filter = pdf_dict_gets(dict, "Filter");
	if (!strcmp(pdf_to_name(filter), "FlateDecode")) 
		return TRUE;
	int n = 0;
	n = pdf_array_len(filter);  
	for (int i = 0; i < n; i++)
		if (!strcmp(pdf_to_name(pdf_array_get(filter, i)), "FlateDecode"))
			return TRUE;
	return FALSE;
}

BOOL  CPdfCompressEx::WritePixmap(fz_context *ctx, fz_pixmap *pix, ATL::CString strImagePath, int rgb)
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

BOOL CPdfCompressEx::SaveImageAsPng(ATL::CString strImagePath, INT32 nNum)
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

	ref = pdf_new_indirect(m_doc, nNum, 0);
	UINT32 u = 0;
	//保存图片到文件
	image = pdf_load_image(m_doc, ref);
	if (!image)
	{
		return bRet;
	}

#if 0
	std::string strBuffer((const char*)image->buffer->buffer->data,image->buffer->buffer->len);

	UINT8* bmpBuf = NULL;
	UINT64 u64BytesOut = 0;
	bmpBuf = Util::Image::CreateRenderedBitmap((const char*)strBuffer.c_str(), image->w, image->h, FALSE, &u64BytesOut);

	CxImage imag2e(bmpBuf, u64BytesOut, CXIMAGE_FORMAT_BMP);
	if (imag2e.IsValid())
	{
		imag2e.SetJpegQuality(50);
		imag2e.Save(_T("E:\\test\\convert\\bitmap.jpg"), CXIMAGE_FORMAT_JPG);
	}
#endif
#if 1
	fz_try(m_ctx)
	{
		pix = fz_new_pixmap_from_image(m_ctx, image, 0, 0);
		if (!pix)
		{
			return bRet;
		}
		fz_drop_image(m_ctx, image);
		//fz_free_image(ctx, &image->storable);
		//rgb mode
		bRet = WritePixmap(m_ctx, pix, strImagePath, 1);

		fz_drop_pixmap(m_ctx, pix);
		pdf_drop_obj(ref);
	}
	fz_catch(m_ctx)
	{
		u = GetLastError();
	}
#endif

	return TRUE;
}


BOOL CPdfCompressEx::SavaImageAsJpg(INT32 nNum)
{

#if 0
	pdf_obj    *ref = pdf_new_indirect(m_doc, nNum, 0);
	if (!ref)
	{
		return FALSE;
	}

	fz_image   *image = pdf_load_image(m_doc, ref);
	if (!image)
	{
		pdf_drop_obj(ref);
		return FALSE;
	}

	fz_buffer* fz_buf = pdf_load_stream(m_doc, nNum, 0);
	if (nullptr == fz_buf || nullptr == fz_buf->data || 0 == fz_buf->len)
	{
		pdf_drop_obj(ref);
		fz_drop_image(m_ctx, image);
		return FALSE;
	} 

#endif
	pdf_xref_entry *entry = nullptr;
	fz_buffer* fz_buf = nullptr;
	UINT32 width = 0, height = 0, bpc =0;
	fz_try(m_ctx)
	{
		entry = pdf_get_xref_entry(m_doc, nNum);

		pdf_obj*  obj = entry->obj;
		width = pdf_to_int(pdf_dict_getsa(obj, "Width", "W"));
		height = pdf_to_int(pdf_dict_getsa(obj, "Height", "H"));
		bpc = pdf_to_int(pdf_dict_getsa(obj, "BitsPerComponent", "BPC"));
		if (bpc == 0)
			bpc = 8;
		UINT32 imagemask = pdf_to_bool(pdf_dict_getsa(obj, "ImageMask", "IM"));

		fz_buf = pdf_load_stream(m_doc, nNum, entry->gen);
		if (nullptr == fz_buf || nullptr == fz_buf->data || 0 == fz_buf->len)
		{
			return FALSE;
		}
	}
	fz_catch(m_ctx)
	{
		UINT32 u = GetLastError();
		return FALSE;
	}

	UINT64 uSizeOut = 0;
	UINT8* bmpBuffer = Util::Image::AssembleBitmap(fz_buf->data, fz_buf->len, width, height, bpc, &uSizeOut);
 	if (nullptr == bmpBuffer){
 		return FALSE;
 	}
#if 0
	//test
	FILE* fp2;
	fp2 = fopen("E:\\test\\convert\\111111.bmp", "wb");
	fwrite(bmpBuffer, uSizeOut, 1, fp2);
	fclose(fp2);
	return TRUE;
#endif

	CxImage ximage((UINT8*)bmpBuffer, uSizeOut, CXIMAGE_FORMAT_BMP);
	if (!ximage.IsValid()){
		return FALSE;
	}

	ximage.SetJpegQuality(50);
	CString strDestImage;
	strDestImage.Format(_T("%s\\yueshu-img-%d.jpg"), Util::Path::GetImageTempPath(), nNum);
	ximage.Save(strDestImage, CXIMAGE_FORMAT_JPG);

	if (bmpBuffer){
		delete[] bmpBuffer;
		bmpBuffer = nullptr;
	}
	//pdf_drop_obj(ref);
	fz_drop_buffer(m_ctx,fz_buf);
//	fz_drop_image(m_ctx, image);
	return TRUE;
}

BOOL CPdfCompressEx::IsWriteStream(INT32 nNum)
{
	BOOL bRet = FALSE;
	//流中点阵图的大小
	fz_buffer* fz_buf = pdf_load_raw_stream(m_doc, nNum, 0);
	if (nullptr == fz_buf)
	{
		ATLASSERT(FALSE);
		return bRet;
	}
	UINT32 uLen = fz_buf->len;
	fz_drop_buffer(m_ctx, fz_buf);

	//获取jpg图片的大小
	CString strDestImage;
	strDestImage.Format(_T("%s\\yueshu-img-%d.jpg"), Util::Path::GetImageTempPath(), nNum);

	HANDLE hFile = CreateFile(strDestImage, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
		return FALSE;
	LARGE_INTEGER size;
	bRet = GetFileSizeEx(hFile, &size);
	if (!bRet)
	{
		return bRet;
	}
	UINT32 uSize = size.QuadPart;
	if (uLen < uSize)
	{
		bRet = FALSE;
	}
	return bRet;
}

//解析资源信息
void CPdfCompressEx::PraseImageTypeObj(INT32 nId, CString strPdfOutPath)
{
	std::vector<UINT32> vecObjNum;
	UINT32 uObjNums = pdf_count_objects(m_doc);
	for (UINT32 uIndex = 1; uIndex < uObjNums; uIndex++)
	{
		pdf_obj  *obj;
		obj = pdf_load_object(m_doc, uIndex, 0);
		if (IsImageType(obj) && IsFlateDecode(obj))
 		{
			vecObjNum.push_back(uIndex);
 		}
		pdf_drop_obj(obj);
	}

	BOOL bPerfectPDF = TRUE;
	static UINT32 uCurIndex = 1;
	for (UINT32 uIndex = 0; uIndex < vecObjNum.size(); uIndex++)
	{	
		UINT32 uObjNum = vecObjNum[uIndex];

		CString strDestImage;
		strDestImage.Format(_T("%s\\yueshu-img-%d.jpg"), Util::Path::GetImageTempPath(), uObjNum);
		
		if (SavaImageAsJpg(uObjNum))
		{
			if (IsWriteStream(uObjNum))
			{
				bPerfectPDF = FALSE;
				pdf_obj  *obj = NULL;
				obj = pdf_load_object(m_doc, uObjNum, 0);
				WriteDataToStream(obj, strDestImage, uObjNum);
				pdf_drop_obj(obj);
			}
		}
		DeleteFile(strDestImage);

		//跳线程设置进度
		CString strOutInfo;
		strOutInfo.Format(_T("Prase image obj:%d"), uObjNum);
		JumpThreadSetProgress(nId, E_PDFCOMPRESS_STATE_PRASE, 100 * (uCurIndex++) / vecObjNum.size(), strOutInfo);
	}

	if (bPerfectPDF)
	{
		CString strOutInfo;
		strOutInfo.Format(_T("Perfect PDF！Not Need Prase!"));
		JumpThreadSetProgress(nId, E_PDFCOMPRESS_STATE_PERFECT, 0, strOutInfo);
	}
	else
	{
		fz_write_options ops = { 0 };
		ATL::CW2A szPdfOutPath(strPdfOutPath, CP_UTF8);
		pdf_write_document(m_doc, (char*)szPdfOutPath, &ops);

		CString strOutInfo;
		strOutInfo.Format(_T("Prase Sucess!"));
		JumpThreadSetProgress(nId, E_PDFCOMPRESS_STATE_FINISH, 100, strOutInfo);
	}
	if (m_doc){
		pdf_close_document(m_doc);
	}
	if (m_ctx){
		fz_free_context(m_ctx);
	}
	m_doc = nullptr;
	m_ctx = nullptr;

	uCurIndex = 1;
}


BOOL  CPdfCompressEx::WriteDataToStream(pdf_obj* dict, ATL::CString  strDestImagePath, INT32 nNum)
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

	fz_buffer * stm_buf = nullptr;
	fz_try(m_ctx)
	{
		//对考jpeg图片格式数据到流中
		stm_buf = fz_new_buffer(m_ctx, (int)uSize);
		memcpy(stm_buf->data, szBuf, uSize);
		stm_buf->len = uSize;
	}
	fz_catch(m_ctx)
	{
		UINT32 u = 0;
		u = GetLastError();
	}
	delete[] szBuf;

#if 0
	CStringA strTest;
	for (UINT32 i = 0; i < uSize; i++)
	{
		strTest.AppendFormat("%02X", szBuf[i]);
	}
	FILE* fp;
	fp = fopen("E:\\1111.txt", "wb");
	fwrite(strTest, strTest.GetLength(), 1, fp);
	fclose(fp);
#endif

	//更新长度
	pdf_obj *length = pdf_dict_gets(dict, "Length");
	if (pdf_is_indirect(length))
	{
		int num = pdf_to_num(length);
		int gen = pdf_to_gen(length);
		pdf_obj* lenobj = pdf_load_object(m_doc, num, gen);
		pdf_set_int(lenobj, uSize);
		pdf_drop_obj(lenobj);
		pdf_update_object(m_doc, num, lenobj);
	}
	else if (pdf_is_int(length))
	{
		pdf_set_int(length, uSize);
	}
	pdf_dict_dels(dict, "Filter");
	pdf_dict_puts_drop(dict, "Filter", pdf_new_name(m_doc, "DCTDecode"));
	pdf_dict_dels(dict, "ColorSpace");
	pdf_dict_dels(dict, "DecodeParms");
	pdf_dict_puts_drop(dict, "ColorSpace", pdf_new_name(m_doc, "DeviceRGB"));
	pdf_dict_puts_drop(dict, "BitsPerComponent", pdf_new_int(m_doc, 8));

	//更新流
	pdf_update_stream(m_doc, nNum, stm_buf);
	fz_drop_buffer(m_ctx, stm_buf);

	return TRUE;
}


void CPdfCompressEx::JumpThreadSetProgress(INT32 nId, E_PDFCOMPRESS_STATE eState, INT32 nVal, CString strOutInfo)
{
	ST_COMPRESS_RESULT* stCompressResult = new ST_COMPRESS_RESULT;
	stCompressResult->nId = nId;
	stCompressResult->eState = eState;
	stCompressResult->nVal = nVal;
	stCompressResult->strOutInfo = strOutInfo;
	::PostMessage(GetMsgWnd(), WM_MYCOMPRESS_UI_TASK, (WPARAM)stCompressResult, (LPARAM)0);
}


void CPdfCompressEx::OnMessage(UINT32 uMsgID, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (WM_MYCOMPRESS_UI_TASK == uMsgID)
	{

		ST_COMPRESS_RESULT* stCompressResult = (ST_COMPRESS_RESULT*)(wParam);
		if (NULL == stCompressResult)
		{
			return;
		}
		if (m_Func_CallBack)
		{
			m_Func_CallBack(stCompressResult->eState, stCompressResult->nId,stCompressResult->nVal, stCompressResult->strOutInfo);
		}
		delete stCompressResult;
	}
}


const char* CPdfCompressEx::GetClass()
{
	return "PdfCompressClass";
}

void CPdfCompressEx::Run()
{
	while (m_bRun)
	{
		// 提取任务
		m_CritSec.Lock();
		if (m_TaskList.empty())
		{
			m_CritSec.Unlock();
			Sleep(1);
			continue;
		}

		auto it = m_TaskList.begin();
		ST_PDFINFO_NODE itPdfInfoNode = *it;
		m_TaskList.erase(it);
		m_CritSec.Unlock();
		
		INT32	nId				= itPdfInfoNode.nId;
		CString strPdfInPath	= itPdfInfoNode.strPdfInPath;
		CString strPdfOutFolder = itPdfInfoNode.strPdfOutFolder;

		CString strPdfOutPath = Util::Path::GetPDFOutPath(strPdfInPath, strPdfOutFolder);
		if (DocumentInit(strPdfInPath, _T("")))
		{
			PraseImageTypeObj(nId,strPdfOutPath);
		}
		else
		{
			//通知该PDF文档初始化失败
			JumpThreadSetProgress(nId, E_PDFCOMPRESS_STATE_FAIL, 0, _T("DocumentInit is Fail"));
		}
		Sleep(1);
	}
}

void CPdfCompressEx::ExistThread(bool bForce)
{
	bool bRet = false;
	m_bRun = FALSE;
	if (m_uThreadId)
	{
		Thread::IThreadManager* pThreadManager = NULL;
		Thread::GetBaseThreadObject((void**)&pThreadManager);
		if (NULL == pThreadManager)
		{
			ATLASSERT(FALSE);
			return;
		}
		pThreadManager->ExistThread(m_uThreadId, bForce);
	}
}

