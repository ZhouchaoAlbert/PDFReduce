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
	BOOL IsImageType(pdf_obj* dict);
	//彩色空间是RGB
	BOOL IsRGBColorSpace(pdf_obj* dict);
	//是zlib压缩
	BOOL IsFlateDecode(pdf_obj* dict);

	//图片的Buffer 二进制流是否压缩
	BOOL IsCompressImageStream(pdf_document* doc,INT32 num,INT32 gen);

	//解析资源图片
	void PraseImageTypeObj();
	//保存图片PNG
	BOOL SaveImageAsPng(ATL::CString strImagePath, INT32 nNum);
	//保存图片格式
	BOOL SavaImageAsJpg(INT32 nNum);
	//PDF中图片压缩格式是FlateDecode 与转换后jpg的图片大小比较
	BOOL IsWriteStream(INT32 nNum);
	//写pixmap
	BOOL WritePixmap(fz_context *ctx, fz_pixmap *pix, ATL::CString strImagePath, int rgb);
	//图片转换
	BOOL ImageConvert(ATL::CString strSrcImagePath, ATL::CString strDestImagePath, INT32 nNum);
	//数据回写
	BOOL WriteDataToStream(pdf_obj* dict, ATL::CString  strDestImagePath, INT32 nNum);

private:
	// 线程回调
	static UINT WINAPI  ThreadProc(void* pVoid);
	void Run();
	void JumpThreadSetProcess(INT32 nCurPos, INT32 nTotal);
	BOOL ImageSizeCompare(CString strSrcIamge, CString strDestImage);
private:
	ATL::CString m_strPdfPath;
	ATL::CString m_strPassword;
	ATL::CString m_strPdfOutPath;
	HWND m_hWnd;
	fz_context_s* m_ctx; 
	pdf_document* m_doc; 
};

