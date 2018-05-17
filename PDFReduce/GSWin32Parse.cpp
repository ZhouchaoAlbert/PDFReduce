#include "GSWin32Parse.h"
#include "UtilPath.h"
#include <vector>

fz_context_s*  CGSWin32Parse::ctx = NULL;
pdf_document* CGSWin32Parse::doc = NULL;

CGSWin32Parse::CGSWin32Parse():
m_hThread(nullptr)
{
	AddMsg(UM_GSWIN32_UI_TASK);
	Start();
}


CGSWin32Parse::~CGSWin32Parse()
{
	ExistThread(true);
	Stop();
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

void CGSWin32Parse::SetProcessCallback(std::function<void(INT32 nCode, INT32 nVal, CString strOutInfo)>Func_CallBack)
{
	m_Func_CallBack = Func_CallBack;
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
	BOOL bRet = FALSE;
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
					CString strOutInfo;
					strOutInfo.Format(_T("Prase:%s"), vecSplit[uIndex]);
					JumpThreadSetProcess(1, uCurProcess++ * 100 / m_uPagesCount, strOutInfo);
					bRet = TRUE;
				}
				UINT32 uFind2 = vecSplit[uIndex].Find(_T("Error:"));
				if (uFind2 == 0)
				{
					bRet = FALSE;
					break; 
				}
			}
		}
	
		CloseHandle(piProcInfo.hProcess);
		CloseHandle(piProcInfo.hThread);
	} while (0);

	if (bRet){
		JumpThreadSetProcess(0, 100, _T("Prase Sucess!"));
	}
	else{
		JumpThreadSetProcess(3, 0, _T("Prase Fail!"));
	}
	uCurProcess = 1;
}

void CGSWin32Parse::OnMessage(UINT32 uMsgID, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (UM_GSWIN32_UI_TASK == uMsgID)
	{
		ST_PARSE_RESULT* stPraseResult = (ST_PARSE_RESULT*)(wParam);
		if (NULL == stPraseResult)
		{
			return;
		}
		if (m_Func_CallBack)
		{
			m_Func_CallBack(stPraseResult->nCode, stPraseResult->nVal, stPraseResult->strOutInfo);
		}
		delete stPraseResult;
	}
}

void CGSWin32Parse::JumpThreadSetProcess(INT32 nCode, INT32 nVal, CString strPraseInfo)
{
	ST_PARSE_RESULT* stPraseResult = new ST_PARSE_RESULT;
	stPraseResult->nCode = nCode;
	stPraseResult->nVal = nVal;
	stPraseResult->strOutInfo = strPraseInfo;
	::PostMessage(GetMsgWnd(), UM_GSWIN32_UI_TASK, (WPARAM)stPraseResult, (LPARAM)0);
}


void CGSWin32Parse::ExistThread(bool bForced)
{
	if (m_hThread)
	{
		if (bForced)
			::TerminateThread(m_hThread, 0);
		else
			::WaitForSingleObject(m_hThread, INFINITE);
		CloseHandle(m_hThread);
	}
	m_hThread = nullptr;
}
