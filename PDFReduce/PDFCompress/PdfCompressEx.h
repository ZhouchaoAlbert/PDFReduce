#ifndef __PDF_COMPRESS_H__
#define __PDF_COMPRESS_H__

#include <atlstr.h>
#include <string>
#include <list>
#include <functional>
#include "..\Msg\UtilMsg.h"
#include "..\Base\BaseCom.h"
#include "..\Thread\ThreadManager.h"
#include "..\Help\MyLock.h"

using namespace ATL;
using namespace std;
extern "C"
{
#include <mupdf/pdf.h>
#include <mupdf/fitz.h>
}

struct ST_PDFINFO_NODE
{
	INT32   nId;						//Ψһ��ʾ  id����
	CString strPdfInPath;				//����PDF·�� 
	CString strPassword;
	CString strPdfOutFolder;			//���PDF�ļ���
};

enum E_PDFCOMPRESS_STATE
{
	E_PDFCOMPRESS_STATE_FINISH  = 0,	//0 ���
	E_PDFCOMPRESS_STATE_PRASE   = 1,	//1 ������
	E_PDFCOMPRESS_STATE_PERFECT = 2,	//2 ����PDF ����Ҫ����
	E_PDFCOMPRESS_STATE_FAIL    = 3,	//3 ����ʧ��
};

struct ST_COMPRESS_RESULT
{
	CString					strPdfInPath;//Ψһ��ʾ
	E_PDFCOMPRESS_STATE     eState;		//ѹ��״̬
	INT32					nVal;		//����ֵ
	CString					strOutInfo; //�����Ϣ
};


class CPdfCompressEx : public Util::Msg::CMsgBase, public Thread::IThreadCallback
{
public:
	CPdfCompressEx();
	~CPdfCompressEx();

	//����������
	void   AddTask(ST_PDFINFO_NODE TaskNode);
	//�����߳̿�ʼѹ��PDF�ļ�
	UINT32 StartCompress();

	//���ý��Ȼص�
	void  SetProcessCallback(std::function<void(INT32 nCode, CString strPdfInPath, INT32 nVal, CString strOutInfo)>Func_CallBack);
	//�˳��߳�
	void ExistThread(bool bForce);

protected:
	//��ʼ������
	BOOL DocumentInit(ATL::CString strPdfPath = _T(""), ATL::CString strPassword = _T(""));

	//��ȡPDFҳ
	UINT32 GetPdfPages(pdf_document* doc);
	//�Ƿ���ͼƬ
	BOOL IsImageType(pdf_obj* dict);
	//��ɫ�ռ���RGB
	BOOL IsRGBColorSpace(pdf_obj* dict);
	//��zlibѹ��
	BOOL IsFlateDecode(pdf_obj* dict);

	//������ԴͼƬ
	void PraseImageTypeObj(CString strPdfInPath, CString strPdfOutPath);
	//����ͼƬPNG
	BOOL SaveImageAsPng(ATL::CString strImagePath, INT32 nNum);
	//����ͼƬ��ʽ
	BOOL SavaImageAsJpg(INT32 nNum);
	//PDF��ͼƬѹ����ʽ��FlateDecode ��ת����jpg��ͼƬ��С�Ƚ�
	BOOL IsWriteStream(INT32 nNum);
	//дpixmap
	BOOL WritePixmap(fz_context *ctx, fz_pixmap *pix, ATL::CString strImagePath, int rgb);

	//���ݻ�д
	BOOL WriteDataToStream(pdf_obj* dict, ATL::CString  strDestImagePath, INT32 nNum);


	//��Ϣ
	virtual void OnMessage(UINT32 uMsgID, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	// ��ȡ������
	virtual const char* GetClass();
	//ִ���߳�
	virtual void Run();
	//���߳�
	void JumpThreadSetProgress(CString strPdfInPath, E_PDFCOMPRESS_STATE eState, INT32 nVal, CString strOutInfo);

	IMPLEMENT_REFCOUNT(CPdfCompressEx);
private:
	BOOL                                  m_bRun;
	UINT32                                m_uThreadId;
	std::list<ST_PDFINFO_NODE>		      m_TaskList;
	CCritSec                              m_CritSec;
	fz_context_s* m_ctx; 
	pdf_document* m_doc; 



	std::function<void(INT32 nCode, CString strPdfInPath, INT32 nVal, CString strOutInfo)>m_Func_CallBack;
};

#endif