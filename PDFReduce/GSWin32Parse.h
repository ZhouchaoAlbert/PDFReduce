#pragma once
#include <atlstr.h>
using namespace ATL;
extern "C"
{
#include <mupdf/pdf.h>
#include <mupdf/fitz.h>
}
#include <functional>

#include"UserDefine.h"
#include <functional>
#include "Msg/UtilMsg.h"


class CGSWin32Parse : public Util::Msg::CMsgBase
{
public:
	CGSWin32Parse();
	~CGSWin32Parse();
public:
	void InitCmdLineParam(CString strSrcPDFPath, CString strDestPDFPath, HWND hWnd);
	UINT32 PipeCmdLine();
	void SetProcessCallback(std::function<void(INT32 nCode, INT32 nVal, CString strOutInfo)>Func_CallBack);

	void ExistThread(bool bForced);
	//ÏûÏ¢
	virtual void OnMessage(UINT32 uMsgID, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
protected:
	BOOL Init(ATL::CString strPdfPath, ATL::CString strPassword);
	static UINT32 WINAPI  ThreadProc(void* pVoid);
	void Run();

	void JumpThreadSetProcess(INT32 nCode, INT32 nVal, CString strPraseInfo);
private:
	HWND m_hWnd;
	CString m_strCmdLineParam;
	static fz_context_s* ctx;
	static pdf_document* doc;
	UINT32 m_uPagesCount{0};
	HANDLE m_hThread;
	std::function<void(INT32 nCode, INT32 nVal, CString strOutInfo)>m_Func_CallBack;
};

