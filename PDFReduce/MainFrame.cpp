#include "MainFrame.h"
#include <atlstr.h>
#include <ShlObj.h>
#include <string>
  
#include "UtilImage.h"
#include "UtilMuPdf.h"
#include "resource.h"

CMainFrame::CMainFrame()
{
}


CMainFrame::~CMainFrame()
{

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
}

void CMainFrame::OnFinalMessage(HWND hWnd)
{

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
}

LRESULT CMainFrame::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	LRESULT hr = 0;

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
	
	//对PDF 压缩体积处理
	Util::MuPdf::StartPdfCompress(strPDFPath, _T(""), strPDFOutPath);
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
	TCHAR szBuffer[MAX_PATH] = { 0 };
	BROWSEINFO bi;
	ZeroMemory(&bi, sizeof(BROWSEINFO));
	bi.hwndOwner = NULL;
	bi.pszDisplayName = szBuffer;
	bi.lpszTitle = _T("从下面选择PDF文件:");
	bi.ulFlags = BIF_BROWSEINCLUDEFILES;
	LPITEMIDLIST idl = SHBrowseForFolder(&bi);
	if (NULL == idl)
	{
		return;
	}
	SHGetPathFromIDList(idl, szBuffer);
	
	CString strPDFPath(szBuffer);
	CEditUI* pEdit = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edt_pdf_in_path")));
	if (pEdit && !strPDFPath.IsEmpty())pEdit->SetText(strPDFPath);
}


