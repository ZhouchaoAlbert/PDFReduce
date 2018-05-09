#pragma once
#include <atlstr.h>
#include <string>

extern "C"
{
	#include <mupdf/pdf.h>
	#include <mupdf/fitz.h>
}

static fz_context_s* ctx = NULL;
static pdf_document* doc = NULL;

namespace Util
{
	namespace MuPdf
	{
		 //初始化操作
		BOOL Init(ATL::CString strPdfPath = _T(""), ATL::CString strPassword = _T(""));
		//获取PDF页
		UINT32 GetPdfPages(pdf_document* doc);
		//是否是图片
		BOOL IsImage(pdf_obj* obj);
		//解析资源信息
		void PraseResImage(std::string szSavePath);
		//保存图片
		BOOL SaveImage(std::string szSavePath, INT32 nNum);
		//写pixmap
		BOOL WritePixmap(fz_context *ctx, fz_pixmap *pix, std::string filepath, int rgb);
		//图片转换
		BOOL ImageConvert(std::string szSavePath, INT32 nNum);
		//数据回写
		BOOL WriteDataToStream(pdf_obj* obj, std::string szSavePath, INT32 nNum);


	}
}

