#include "GSWin32Parse.h"
#include "UtilPath.h"
#include <vector>

fz_context_s*  CGSWin32Parse::ctx = NULL;
pdf_document* CGSWin32Parse::doc = NULL;

CGSWin32Parse::CGSWin32Parse()
{
}


CGSWin32Parse::~CGSWin32Parse()
{
}


//初始化操作
BOOL CGSWin32Parse::Init(ATL::CString strPdfPath, ATL::CString strPassword)
{
	if (strPdfPath.IsEmpty())
	{
		ATLASSERT(FALSE);
		return FALSE;
	}
	ATL::CW2A szPdfPath(strPdfPath.GetString(), CP_UTF8);
	ATL::CW2A szPassword(strPassword.GetString(), CP_UTF8);
	//创建上下文 包含全局信息
	ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
	if (!ctx)
	{
		ATLASSERT(FALSE);
		return FALSE;
	}
	//打开文档
	doc = pdf_open_document_no_run(ctx, szPdfPath.m_psz);
	if (!doc)
	{
		ATLASSERT(FALSE);
		return FALSE;
	}
	//PDF是否需要密码及密码鉴权
	if (pdf_needs_password(doc) && !pdf_authenticate_password(doc, szPassword.m_psz))
	{
		ATLASSERT(FALSE);
		return FALSE;
	}




	m_uPagesCount = pdf_count_pages(doc);
	return TRUE;
}


void CGSWin32Parse::InitCmdLineParam(CString strSrcPDFPath, CString strDestPDFPath, HWND hWnd)
{
	if (!Util::Path::IsFileExist(strSrcPDFPath) /*|| !Util::Path::IsFileExist(strDestPDFPath)*/)
	{
		return;
	}
	if (0 != strSrcPDFPath.Right(4).CompareNoCase(_T(".pdf")) || 0 != strDestPDFPath.Right(4).CompareNoCase(_T(".pdf")))
	{
		return;
	}

	//路径转换处理

	UINT32 uFind = strSrcPDFPath.ReverseFind('\\');
	if (uFind < 0)
	{
		return;
	}
	CString strFile = strSrcPDFPath.Right(strSrcPDFPath.GetLength() - uFind - 1);
	CString strTempPDFPath;
	strTempPDFPath.Format(_T("%s\\%s"), Util::Path::GetPDFTempPath(TRUE), strFile);

	//初始化获取 总页数
	Init(strSrcPDFPath, _T(""));
	{
		ATL::CW2A szPdfOutPath(strTempPDFPath, CP_UTF8);
		fz_write_options opts = { 0 };
		opts.do_incremental = 0;
		opts.do_garbage = 3;
		pdf_write_document(doc, (char*)szPdfOutPath, nullptr);
		if (doc)
		{
			pdf_close_document(doc);
			doc = NULL;
			fz_free_context(ctx);
			ctx = NULL;
		}
	}

	strSrcPDFPath = strTempPDFPath;

	CString strGSWin32ExePath;
	strGSWin32ExePath.Format(_T("\"%s\\gswin32c.exe\""), Util::Path::GetExePath());

	strGSWin32ExePath.AppendFormat(_T(" -sDEVICE=pdfwrite -dMaxSubsetPct=100 -dPDFSETTINGS=/ebook -sOutputFile="));
	strGSWin32ExePath.AppendFormat(_T("\"%s\""), strDestPDFPath);
	strGSWin32ExePath.AppendFormat(_T(" -dNOPAUSE -dBATCH "));
	strGSWin32ExePath.AppendFormat(_T("\"%s\""), strSrcPDFPath);
	m_strCmdLineParam = strGSWin32ExePath;
	m_hWnd = hWnd;
}

UINT32 CGSWin32Parse::PipeCmdLine()
{
	UINT32  uiThreadId = 0;
	HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadProc, (void*)this, 0, &uiThreadId);
	return uiThreadId;
}

UINT32 WINAPI  CGSWin32Parse::ThreadProc(void* pVoid)
{
	CGSWin32Parse *pThreadCallback = static_cast<CGSWin32Parse *>(pVoid);
	if (pThreadCallback)
	{
		pThreadCallback->Run();
	}
	return 0;
}


void CGSWin32Parse::Run(){
	
	static UINT32 uCurProcess = 1;
	do
	{
		SECURITY_ATTRIBUTES sa;
		ZeroMemory(&sa, sizeof(sa));
		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.lpSecurityDescriptor = NULL;  //默认的安全描述符   
		sa.bInheritHandle = TRUE;        //这个必须要设定TRUE 
		HANDLE hRead = NULL, hWrite = NULL;
		if (!CreatePipe(&hRead, &hWrite, &sa, 0))
		{
			ATLASSERT(FALSE);
			break;
		}

		STARTUPINFO siStartInfo;
		PROCESS_INFORMATION piProcInfo;
		ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
		siStartInfo.cb = sizeof(STARTUPINFO);
		siStartInfo.dwFlags |= STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
		siStartInfo.hStdOutput = hWrite;
		siStartInfo.hStdError = hWrite;

		if (!CreateProcess(NULL, (LPTSTR)m_strCmdLineParam.GetString(), NULL, NULL, TRUE, NULL, NULL, NULL, &siStartInfo, &piProcInfo))
		{
			ATLASSERT(FALSE);
			break;
		}
		CloseHandle(hWrite);

		//Read
		char		buf[MAX_PATH];
	
		while (1)
		{
			DWORD bytesRead = 0;
			if (!ReadFile(hRead, buf, sizeof(buf) - 1, &bytesRead, NULL) || !bytesRead)
			{
				break;
			}
			buf[bytesRead - 1] = 0;
			CStringA strTemp(buf);
			ATL::CA2W szTemp(strTemp, CP_UTF8);


			std::vector<CString> vecSplit;
			CString strPraseLine(szTemp);
			Util::Path::StringSplit(strPraseLine, _T("\n"), vecSplit);
			for (UINT32 uIndex = 0; uIndex < vecSplit.size();uIndex++)
			{
				UINT32 uFind = vecSplit[uIndex].Find(_T("Page"));
				if (uFind == 0)
				{
					ST_PARSE_RESULT* pParseResult = new ST_PARSE_RESULT;
					pParseResult->uProcess = uCurProcess * 100 / m_uPagesCount;
					pParseResult->strLine = vecSplit[uIndex];
					::PostMessage(m_hWnd, UM_GSWIN32_UI_TASK, (WPARAM)pParseResult, (LPARAM)0);
					uCurProcess++;
				}
			}
		}
	
		CloseHandle(piProcInfo.hProcess);
		CloseHandle(piProcInfo.hThread);
	} while (0);

	if (uCurProcess > 1)
	{
		ST_PARSE_RESULT* pParseResult = new ST_PARSE_RESULT;
		pParseResult->uProcess = 100;
		pParseResult->strLine = _T("Prase Sucess!");
		::PostMessage(m_hWnd, UM_GSWIN32_UI_TASK, (WPARAM)pParseResult, (LPARAM)0);
		uCurProcess = 1;
	}
}
