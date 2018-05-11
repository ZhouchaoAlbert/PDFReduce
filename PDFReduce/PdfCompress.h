#pragma once
#include <atlstr.h>
#include <string>

extern "C"
{
#include <mupdf/pdf.h>
#include <mupdf/fitz.h>
}

class CPdfCompress
{
public:
	CPdfCompress();
	~CPdfCompress();

	UINT32 StartThread(ATL::CString strPdfPath, ATL::CString strPassword, ATL::CString strPdfOutPath);
protected:
	//初始化操作
	BOOL Init(ATL::CString strPdfPath = _T(""), ATL::CString strPassword = _T(""));
	//获取PDF页
	UINT32 GetPdfPages(pdf_document* doc);
	//是否是图片
	BOOL IsImage(pdf_obj* obj);
	//彩色空间是RGB
	BOOL IsRGBColorSpace(pdf_obj* obj);
	//是zlib压缩
	BOOL IsFlateDecode(pdf_obj* obj);

	//图片的Buffer 二进制流是否压缩
	BOOL IsCompressImageStream(pdf_document* doc,INT32 num,INT32 gen);

	//解析资源信息
	void PraseResImage(std::string szSavePath);
	//保存图片
	BOOL SaveImage(std::string szSavePath, INT32 nNum);
	//写pixmap
	BOOL WritePixmap(fz_context *ctx, fz_pixmap *pix, std::string filepath, int rgb);
	//图片转换
	BOOL ImageConvert(CString strSrcImagePath, CString strDestImagePath, INT32 nNum);
	//数据回写
	BOOL WriteDataToStream(pdf_obj* obj, CString  strDestImagePath, INT32 nNum);

private:
	// 线程回调
	static UINT WINAPI  ThreadProc(void* pVoid);
	void Run();
private:
	ATL::CString m_strPdfPath;
	ATL::CString m_strPassword;
	ATL::CString m_strPdfOutPath;

	static fz_context_s* ctx; 
	static pdf_document* doc; 
};

