#pragma once
#include <atlstr.h>
using namespace ATL;
extern "C"
{
#include <mupdf/pdf.h>
#include <mupdf/fitz.h>
}


#define  UM_GSWIN32_UI_TASK WM_USER + 1002



struct ST_PARSE_RESULT
{
	UINT32       uProcess;
	ATL::CString strLine;
};

class CGSWin32Parse
{
public:
	CGSWin32Parse();
	~CGSWin32Parse();
public:
	void InitCmdLineParam(CString strSrcPDFPath, CString strDestPDFPath, HWND hWnd);
	UINT32 PipeCmdLine();


protected:
	BOOL Init(ATL::CString strPdfPath, ATL::CString strPassword);
	static UINT32 WINAPI  ThreadProc(void* pVoid);
	void Run();
private:
	HWND m_hWnd;
	CString m_strCmdLineParam;
	static fz_context_s* ctx;
	static pdf_document* doc;
	UINT32 m_uPagesCount{0};
};

