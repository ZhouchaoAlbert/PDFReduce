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
	//��ʼ������
	BOOL Init(ATL::CString strPdfPath = _T(""), ATL::CString strPassword = _T(""));
	//��ȡPDFҳ
	UINT32 GetPdfPages(pdf_document* doc);
	//�Ƿ���ͼƬ
	BOOL IsImage(pdf_obj* obj);
	//��ɫ�ռ���RGB
	BOOL IsRGBColorSpace(pdf_obj* obj);

	BOOL IsFlateDecode(pdf_obj* obj);

	//������Դ��Ϣ
	void PraseResImage(std::string szSavePath);
	//����ͼƬ
	BOOL SaveImage(std::string szSavePath, INT32 nNum);
	//дpixmap
	BOOL WritePixmap(fz_context *ctx, fz_pixmap *pix, std::string filepath, int rgb);
	//ͼƬת��
	BOOL ImageConvert(std::string szSavePath, INT32 nNum);
	//���ݻ�д
	BOOL WriteDataToStream(pdf_obj* obj, std::string szSavePath, INT32 nNum);

private:
	// �̻߳ص�
	static UINT WINAPI  ThreadProc(void* pVoid);
	void Run();
private:
	ATL::CString m_strPdfPath;
	ATL::CString m_strPassword;
	ATL::CString m_strPdfOutPath;

	static fz_context_s* ctx; 
	static pdf_document* doc; 
};

