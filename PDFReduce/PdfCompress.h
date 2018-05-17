#pragma once
#include <atlstr.h>
#include <string>
#include "MyLock.h"
#include "Msg\UtilMsg.h"
#include <functional>
#include "UserDefine.h"


extern "C"
{
#include <mupdf/pdf.h>
#include <mupdf/fitz.h>
}

class CPdfCompress : public Util::Msg::CMsgBase
{
public:
	CPdfCompress();
	~CPdfCompress();
	static CPdfCompress* GetObj()
	{ 
		static CPdfCompress obj;
		return &obj;
	}
	UINT32 StartThread(ATL::CString strPdfPath, ATL::CString strPassword, ATL::CString strPdfOutPath);
	void SetProcessCallback(std::function<void(INT32 nCode, INT32 nVal, CString strOutInfo)>Func_CallBack);
	//�˳��߳�
	void ExistThread(bool bForced);
protected:
	//��ʼ������
	BOOL Init(ATL::CString strPdfPath = _T(""), ATL::CString strPassword = _T(""));
	//��ȡPDFҳ
	UINT32 GetPdfPages(pdf_document* doc);
	//�Ƿ���ͼƬ
	BOOL IsImageType(pdf_obj* dict);
	//��ɫ�ռ���RGB
	BOOL IsRGBColorSpace(pdf_obj* dict);
	//��zlibѹ��
	BOOL IsFlateDecode(pdf_obj* dict);

	//ͼƬ��Buffer ���������Ƿ�ѹ��
	BOOL IsCompressImageStream(pdf_document* doc,INT32 num,INT32 gen);

	//������ԴͼƬ
	void PraseImageTypeObj();
	//����ͼƬPNG
	BOOL SaveImageAsPng(ATL::CString strImagePath, INT32 nNum);
	//����ͼƬ��ʽ
	BOOL SavaImageAsJpg(INT32 nNum);
	//PDF��ͼƬѹ����ʽ��FlateDecode ��ת����jpg��ͼƬ��С�Ƚ�
	BOOL IsWriteStream(INT32 nNum);
	//дpixmap
	BOOL WritePixmap(fz_context *ctx, fz_pixmap *pix, ATL::CString strImagePath, int rgb);
	//ͼƬת��
	BOOL ImageConvert(ATL::CString strSrcImagePath, ATL::CString strDestImagePath, INT32 nNum);
	//���ݻ�д
	BOOL WriteDataToStream(pdf_obj* dict, ATL::CString  strDestImagePath, INT32 nNum);


	//��Ϣ
	virtual void OnMessage(UINT32 uMsgID, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
private:
	// �̻߳ص�
	static UINT WINAPI  ThreadProc(void* pVoid);
	void Run();
	void JumpThreadSetProcess(INT32 nCode,INT32 nVal,CString strPraseInfo);

private:
	ATL::CString m_strPdfPath;
	ATL::CString m_strPassword;
	ATL::CString m_strPdfOutPath;
	fz_context_s* m_ctx; 
	pdf_document* m_doc; 
	HANDLE   m_hThread;
	CCritSec m_CritSec;
	std::function<void(INT32 nCode,INT32 nVal,CString strOutInfo)>m_Func_CallBack;
};

