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
	//��ʼ������
	BOOL Init(ATL::CString strPdfPath = _T(""), ATL::CString strPassword = _T(""));
	//��ȡPDFҳ
	UINT32 GetPdfPages(pdf_document* doc);
	//�Ƿ���ͼƬ
	BOOL IsImage(pdf_obj* obj);
	//��ɫ�ռ���RGB
	BOOL IsRGBColorSpace(pdf_obj* obj);
	//��zlibѹ��
	BOOL IsFlateDecode(pdf_obj* obj);

	//ͼƬ��Buffer ���������Ƿ�ѹ��
	BOOL IsCompressImageStream(pdf_document* doc,INT32 num,INT32 gen);

	//������Դ��
	void PraseResImage();
	//����ͼƬ
	BOOL SaveImage(ATL::CString strImagePath, INT32 nNum);
	//дpixmap
	BOOL WritePixmap(fz_context *ctx, fz_pixmap *pix, ATL::CString strImagePath, int rgb);
	//ͼƬת��
	BOOL ImageConvert(ATL::CString strSrcImagePath, ATL::CString strDestImagePath, INT32 nNum);
	//���ݻ�д
	BOOL WriteDataToStream(pdf_obj* obj, ATL::CString  strDestImagePath, INT32 nNum);

private:
	// �̻߳ص�
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

