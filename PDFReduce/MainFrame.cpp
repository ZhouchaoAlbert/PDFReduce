#include "MainFrame.h"
#include <atlstr.h>
#include <ShlObj.h>
#include <string>
  
#include "UtilImage.h"
#include "UtilMuPdf.h"
#include "resource.h"
#include "DropTarget/DropTargetReal.h"
#include "Singleton.h"
#include "PdfCompress.h"
#include "UtilPath.h"

CMainFrame::CMainFrame():
m_uCompressMode(1)
{
}


CMainFrame::~CMainFrame()
{
	if (1 == m_uCompressMode)
	{
		Singleton<CPdfCompress>::UnInstance();
	}
	else if (2 == m_uCompressMode)
	{
		Singleton<CGSWin32Parse>::UnInstance();
	}
}


CDuiString CMainFrame::GetSkinFolder()
{
	return _T("Resource");
}

CDuiString CMainFrame::GetSkinFile()
{
	return _T("xml\\MainFrame.xml");
}

LPCTSTR CMainFrame::GetWindowClassName(void) const
{
	return _T("MainFrameClass");
}

CControlUI* CMainFrame::CreateControl(LPCTSTR pstrClass)
{
	return NULL;
}

void CMainFrame::InitWindow()
{
	m_hIcon = LoadIcon(CPaintManagerUI::GetInstance(), MAKEINTRESOURCE(IDI_ICON1));
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);
	m_pTreeList = static_cast<CTabLayoutUI*>(m_PaintManager.FindControl(_T("tab_tree_list")));
	
	CEditUI* pEdit = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edt_pdf_in_path")));
	pEdit->SetText(_T("E:\\test\\convert\\Desert.pdf"));
	CEditUI* pEditOut = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edt_pdf_out_path")));
	pEditOut->SetText(_T("E:\\test\\convert\\out.pdf"));
	
	//Singleton<CDropTargetReal>::Instance().RegisterDropTarget(m_hWnd, this);
	::DragAcceptFiles(m_hWnd, true);
	
}

void CMainFrame::OnFinalMessage(HWND hWnd)
{
//	Singleton<CDropTargetReal>::Instance().RevokeDropTarget(m_hWnd);
}

LRESULT CMainFrame::ResponseDefaultKeyEvent(WPARAM wParam)
{
	if (wParam == VK_RETURN)
	{
		return TRUE;
	}
	else if (wParam == VK_F10)
	{
		return TRUE;
	}


	return FALSE;
}

void CMainFrame::Notify(TNotifyUI& msg)
{
	if (msg.sType == DUI_MSGTYPE_CLICK){
		CDuiString szName = msg.pSender->GetName();
		if (szName == _T("btn_min"))
		{
			SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);
		}
		else if (szName == _T("btn_close"))
		{
			ShowWindow(false);
			PostQuitMessage(0);

		}
		else if (szName == _T("opt_pdf_reduce"))  //select pdf reduce
		{
			m_pTreeList->SelectItem(0);
		}
		else if (szName == _T("opt_pic_convert"))  //select pic convert
		{
			m_pTreeList->SelectItem(1);
		}
		else if (szName == _T("opt_pdf_author"))  //select pdf author
		{
			m_pTreeList->SelectItem(2);
		}
		else if (szName == _T("btn_open_pdf_compress"))  //启动PDF体积压缩
		{
			StartPDFCompress();
		}
		else if (szName == _T("btn_start_pic_convert"))  //开始图片转换
		{
			StartPicConvert();
		}
		else if (szName == _T("btn_open_pdf_path"))  //选择打开PDF路径框
		{
			SelectPDFFolderDialog();
		}

	}
	else if(msg.sType == DUI_MSGTYPE_ITEMSELECT)
	{
		CDuiString szName = msg.pSender->GetName();
		if (szName == _T("comb_group"))
		{
			if (msg.wParam == 0){
				m_uCompressMode = 1;
			}
			else if (msg.wParam == 1){
				m_uCompressMode = 2;
			}
		}
	}
}

LRESULT CMainFrame::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HRESULT hr = S_OK;


	switch (uMsg)
	{
	case WM_DROPFILES:
		hr = OnDropFiles(uMsg, wParam, lParam, bHandled);
		break;
	default:
		break;
	}

	if (bHandled) return hr;
	return __super::HandleCustomMessage(uMsg, wParam, lParam, bHandled);
}


BOOL IsFileExist(const CString& csFile)
{
	DWORD dwAttrib = GetFileAttributes(csFile);
	return INVALID_FILE_ATTRIBUTES != dwAttrib && 0 == (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}


void CMainFrame::StartPDFCompress()
{
	CEditUI* pEdit = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edt_pdf_in_path")));
	ATL::CString strPDFPath = pEdit->GetText();
	if (strPDFPath.IsEmpty())
	{
		::MessageBox(m_hWnd, _T("输入PDF路径为空,请输入PDF路径!"), _T("提示"), MB_OK);
		return;
	}

	if (!IsFileExist(strPDFPath))
	{
		::MessageBox(m_hWnd, _T("输入PDF路径不存在，请重新输入!"), _T("提示"), MB_OK);
		return;
	}

	CString strExe = strPDFPath.Right(4);
	if (0 != strExe.CompareNoCase(_T(".pdf")))
	{
		::MessageBox(m_hWnd, _T("输入的不是PDF文件，请重新输入!"), _T("提示"), MB_OK);
		return;
	}

	CEditUI* pEditOut = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edt_pdf_out_path")));
	ATL::CString strPDFOutPath = pEditOut->GetText();
	CString strExeOut = strPDFOutPath.Right(4);
	if (0 != strExeOut.CompareNoCase(_T(".pdf")))
	{
		::MessageBox(m_hWnd, _T("输出的不是PDF文件，请重新输入!"), _T("提示"), MB_OK);
		return;
	}
	CProgressUI* pProgress = static_cast<CProgressUI*>(m_PaintManager.FindControl(_T("progress")));
	if (pProgress)
	{
		pProgress->SetValue(0);
	}

	if (1 == m_uCompressMode)
	{
		//对PDF 压缩体积处理
		Singleton<CPdfCompress>::Instance().SetProcessCallback(
			[this](INT32 nCode,INT32 nVal,CString strOutInfo){	
			if (0 == nCode || 1 == nCode)
			{
				CProgressUI* pProgress = static_cast<CProgressUI*>(m_PaintManager.FindControl(_T("progress")));
				if (pProgress){
					pProgress->SetValue(nVal);
				}		
				CLabelUI* pParseTips = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("label_parse_tips")));
				if (pParseTips){
					pParseTips->SetText(strOutInfo);
				}
			}
			else if (2 == nCode)  
			{
				CLabelUI* pParseTips = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("label_parse_tips")));
				if (pParseTips){
					pParseTips->SetText(strOutInfo);
				}
			}
		});
		Singleton<CPdfCompress>::Instance().StartThread(strPDFPath, _T(""), strPDFOutPath);
	}
	else if (2 == m_uCompressMode)
	{
		Singleton<CGSWin32Parse>::Instance().SetProcessCallback(
			[this](INT32 nCode, INT32 nVal, CString strOutInfo){
			if (0 == nCode || 1 == nCode)
			{
				CProgressUI* pProgress = static_cast<CProgressUI*>(m_PaintManager.FindControl(_T("progress")));
				if (pProgress){
					pProgress->SetValue(nVal);
				}
				CLabelUI* pParseTips = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("label_parse_tips")));
				if (pParseTips){
					pParseTips->SetText(strOutInfo);
				}
			}
			else if (2 == nCode)
			{
				CLabelUI* pParseTips = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("label_parse_tips")));
				if (pParseTips){
					pParseTips->SetText(strOutInfo);
				}
			}
		});
		Singleton<CGSWin32Parse>::Instance().InitCmdLineParam(strPDFPath, strPDFOutPath, m_hWnd);
		Singleton<CGSWin32Parse>::Instance().PipeCmdLine();
	}
}


void CMainFrame::StartPicConvert()
{

	CEditUI* pEditIn = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edt_pic_in_path")));
	ATL::CString strPicPath = pEditIn->GetText();
	if (strPicPath.IsEmpty())
	{
		::MessageBox(m_hWnd, _T("输入图片路径为空,请输入图片路径!"), _T("提示"), MB_OK);
		return;
	}

	if (!IsFileExist(strPicPath))
	{
		::MessageBox(m_hWnd, _T("输入图片路径不存在，请重新输入!"), _T("提示"), MB_OK);
		return;
	}

	CEditUI* pEditOut = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edt_pic_out_path")));
	ATL::CString strPicOutPath = pEditOut->GetText();
	if (strPicOutPath.IsEmpty())
	{
		::MessageBox(m_hWnd, _T("输出图片路径为空,请输入输出图片路径!"), _T("提示"), MB_OK);
		return;
	}
	Util::Image::ConvertType(strPicPath, strPicOutPath);
}

void CMainFrame::SelectPDFFolderDialog()
{

	CString	szFilter = _T("文件(*.pdf)|*.pdf||");
	CFileDialogEx dlg;
	POSITION position;
	CString	szFileName;
	vector<CString>	vecFileList;

	dlg.SetFilter(szFilter);
	dlg.SetFlags(OFN_FILEMUSTEXIST | OFN_EXPLORER);
	dlg.SetMultiSel(TRUE);
	dlg.SetFileNameBufferSize(1024);
	dlg.SetParentWnd(m_hWnd);
	if (!dlg.ShowOpenFileDlg()) return;

	position = dlg.GetStartPosition();
	while (position != NULL)
	{
		szFileName = dlg.GetNextPathName(position);
		vecFileList.push_back(szFileName);
	}
	if (vecFileList.size() > 2)
	{
		MessageBox(NULL, _T("请添加一个PDF文件！"), _T("提示"), MB_OK);
		return;
	}
	
	CString strPDFPath(vecFileList[0]);
	CEditUI* pEdit = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edt_pdf_in_path")));
	if (pEdit && !strPDFPath.IsEmpty())pEdit->SetText(strPDFPath);
}


LRESULT CMainFrame::OnDropFiles(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HDROP hDropInfo = (HDROP)wParam;
	INT	nFileCount = ::DragQueryFile(hDropInfo, (UINT)-1, NULL, 0);
	if (nFileCount > 2)
	{
		MessageBox(NULL, _T("请托一个PDF文件放到改区域！"), _T("提示"),MB_OK);
		return S_OK;
	}

	POINT point;
	::GetCursorPos(&point);
	ScreenToClient(m_hWnd, &point);
	CButtonUI* pControlUI = static_cast<CButtonUI*>(m_PaintManager.FindControl(point));
	if (NULL == pControlUI)
	{
		return S_OK;
	}
	CString strName = pControlUI->GetName();
	if (0 != strName.CompareNoCase(_T("btn_drop")))
	{
		return S_OK;
	}


	BOOL bIsCopyFile = (BOOL)lParam;
	vector<CString> vecFileList;
	for (int i = 0; i < nFileCount; i++)
	{
		TCHAR szFileName[MAX_PATH] = { 0 };
		ZeroMemory(szFileName, sizeof(szFileName));
		::DragQueryFile(hDropInfo, i, szFileName, MAX_PATH);
		vecFileList.push_back(szFileName);
	}
	::DragFinish(hDropInfo);
	if (vecFileList.size() > 0)
	{
		CString strPDFPath(vecFileList[0]);
		if (0 == strPDFPath.Right(4).CompareNoCase(_T(".pdf")))
		{
			CEditUI* pEdit = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edt_pdf_in_path")));
			if (pEdit && !strPDFPath.IsEmpty())pEdit->SetText(strPDFPath);
		}
	}
	return S_OK;
}


HRESULT CMainFrame::OnDropEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	return m_PaintManager.OnDropEnter(pDataObj, grfKeyState, pt, pdwEffect);
}
HRESULT CMainFrame::OnDropOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	return m_PaintManager.OnDropOver(grfKeyState, pt, pdwEffect);
}
HRESULT CMainFrame::OnDropLeave()
{
	return m_PaintManager.OnDropLeave();
}
HRESULT CMainFrame::OnDrop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	return m_PaintManager.OnDrop(pDataObj, grfKeyState, pt, pdwEffect);
}
