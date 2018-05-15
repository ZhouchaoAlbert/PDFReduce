#pragma once
#include <atlstr.h>
#include <string>

#define  WM_UI_PROCESS WM_USER + 1000

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
	static CPdfCompress* GetObj()
	{ 
		static CPdfCompress obj;
		return &obj;
	}
	UINT32 StartThread(ATL::CString strPdfPath, ATL::CString strPassword, ATL::CString strPdfOutPath, HWND hWnd);
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

	//解析资源信
	void PraseResImage();
	//保存图片
	BOOL SaveImage(ATL::CString strImagePath, INT32 nNum);
	//写pixmap
	BOOL WritePixmap(fz_context *ctx, fz_pixmap *pix, ATL::CString strImagePath, int rgb);
	//图片转换
	BOOL ImageConvert(ATL::CString strSrcImagePath, ATL::CString strDestImagePath, INT32 nNum);
	//数据回写
	BOOL WriteDataToStream(pdf_obj* obj, ATL::CString  strDestImagePath, INT32 nNum);

private:
	// 线程回调
	static UINT WINAPI  ThreadProc(void* pVoid);
	void Run();
	void JumpThreadSetProcess(INT32 i, INT32 j);
	BOOL ImageSizeCompare(CString strSrcIamge, CString strDestImage);
private:
	ATL::CString m_strPdfPath;
	ATL::CString m_strPassword;
	ATL::CString m_strPdfOutPath;
	HWND m_hWnd;
	static fz_context_s* ctx; 
	static pdf_document* doc; 
	CRITICAL_SECTION ctxAccess;
};

