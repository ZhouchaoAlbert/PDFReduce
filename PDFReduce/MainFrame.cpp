#include "MainFrame.h"
#include <atlstr.h>
#include <ShlObj.h>
#include <string>

#include "resource.h"
#include "Base/Singleton.h"

#include "PDFCompress/PdfCompressEx.h"


CMainFrame::CMainFrame():
m_uCompressMode(1)
{
}


CMainFrame::~CMainFrame()
{
	Singleton<CPdfCompressEx>::Instance().ExistThread(FALSE);
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
	
	m_pHorFileList = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("hor_file_list")));
	m_pHorFileDrop = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("hor_file_drop")));
	m_pFileList = static_cast<CListUI*>(m_PaintManager.FindControl(_T("file_list")));


	CEditUI* pEdit = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edt_pdf_in_path")));
	if (pEdit)pEdit->SetText(_T("E:\\test\\convert\\Desert.pdf"));
	CEditUI* pEditOut = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edt_pdf_out_path")));
	if (pEditOut)pEditOut->SetText(_T("E:\\test\\convert"));

	::DragAcceptFiles(m_hWnd, true);
	
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
		else if (szName == _T("btn_drop"))
		{
			SelectPDFFolderDialog();
		}
		else if (szName == _T("btn_out_directory"))
		{
			SelectOutDirectory();
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
	CButtonUI* pStartCompress = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btn_open_pdf_compress")));
	UINT32 uTag = pStartCompress->GetTag();
	if (1 == uTag)
	{
		MessageBox(NULL, _T("PDF压缩任务还在执行,请不要重入!"), _T("提示"), MB_OK);
		return;
	}
	pStartCompress->SetTag(1);

	CString strPDFOutFolder;
	CEditUI* pOutPath = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edt_pdf_out_path")));
	if (NULL != pOutPath)
	{
		strPDFOutFolder = pOutPath->GetText();
	}

	for (UINT32 uIndex = 0; uIndex < m_pFileList->GetCount();uIndex++)
	{
		CListContainerElementUI* pItem = (CListContainerElementUI*)m_pFileList->GetItemAt(uIndex);
		if (NULL  == pItem){
			continue;
		}
		CControlUI* pNode = (CControlUI*)pItem->GetTag();
		if (NULL == pNode){
			continue;
		}
		CString strPDFPath = 	pNode->GetText();
		UINT32 uId = pNode->GetTag();
		if (!IsFileExist(strPDFPath))
		{
			continue;
		}

		//对PDF 压缩体积处理
		Singleton<CPdfCompressEx>::Instance().SetProcessCallback(
			[this](INT32 nCode, INT32 nId,INT32 nVal, CString strOutInfo){

			CListContainerElementUI* pItem = (CListContainerElementUI*)m_pFileList->GetItemAt(nId);
			if (NULL == pItem){
				return;
			}
			CLabelUI* pLabel = (CLabelUI*)pItem->FindSubControl(_T("lab_state"));

			if (E_PDFCOMPRESS_STATE_FINISH == nCode)
			{
				CButtonUI* pStartCompress = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("btn_open_pdf_compress")));
				if (pStartCompress)pStartCompress->SetTag(0);
				CString strTest;
				strTest.Format(_T("压缩完成"));
				pLabel->SetText(strTest);
			}
			else if (E_PDFCOMPRESS_STATE_PRASE == nCode)
			{
				CString strTest;
				strTest.Format(_T("解析进度%d%%"), nVal);
				pLabel->SetText(strTest);
			}
			else if (E_PDFCOMPRESS_STATE_PERFECT == nCode)
			{
				CString strTest;
				strTest.Format(_T("检测是完美的PDF"));
				pLabel->SetText(strTest);
			}
			else if (E_PDFCOMPRESS_STATE_FAIL == nCode)
			{
				CString strTest;
				strTest.Format(_T("压缩失败"));
				pLabel->SetText(strTest);
			}
		});

		ST_PDFINFO_NODE st;
		st.nId				= uId;
		st.strPdfInPath		= strPDFPath;
		st.strPdfOutFolder	= strPDFOutFolder;

		Singleton<CPdfCompressEx>::Instance().AddTask(st);
		Singleton<CPdfCompressEx>::Instance().StartCompress();
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
	//Util::Image::ConvertType(strPicPath, strPicOutPath);
}

void CMainFrame::AddListItem(vector<CString> vecFileList)
{
	m_pHorFileDrop->SetVisible(false);
	m_pHorFileList->SetVisible(true);

	for (UINT32 uIndex = 0; uIndex < vecFileList.size(); uIndex++)
	{
		CString strPDFPath(vecFileList[uIndex]);
		if (0 == strPDFPath.Right(4).CompareNoCase(_T(".pdf")))
		{
			INT32 iStart = strPDFPath.ReverseFind(_T('\\'));
			if (iStart < 0)
			{
				continue;
			}
			if (0 != strPDFPath.Right(4).CompareNoCase(_T(".pdf")))
			{
				continue;
			}
			CString strPdfFileName;
			strPdfFileName = strPDFPath.Mid(iStart + 1, strPDFPath.GetLength() - iStart - 1);


			HANDLE hFile = CreateFile(strPDFPath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
			if (hFile == INVALID_HANDLE_VALUE)
				continue;
			LARGE_INTEGER size;
			BOOL bRet = GetFileSizeEx(hFile, &size);
			if (!bRet)
			{
				continue;
			}
			UINT32 uSize = size.QuadPart;

			ST_LISTITEM_INFO list;
			list.strPDFPath = strPDFPath;
			list.strPDFName = strPdfFileName;
			CString strFileSize;
			strFileSize.Format(_T("%dM"), uSize / 1024 / 1024);
			list.strFileSize = strFileSize;
			list.strState = _T("还未压缩");
			CListContainerElementUI* pItem = GetListItem(list);
			if (pItem)
			{
				m_pFileList->Add(pItem);
			}
		}
	}

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
	if (vecFileList.size() <= 0)
	{
		MessageBox(NULL, _T("请添加一个PDF文件！"), _T("提示"), MB_OK);
		return;
	}
	
	//添加到list
	AddListItem(vecFileList);
}

void CMainFrame::SelectOutDirectory()
{
	BROWSEINFO bi;						// BROWSEINFO结构体
	TCHAR szBuffer[512] = {0};
	TCHAR szFullPath[512] = { 0 };

	bi.hwndOwner = m_hWnd;				// m_hWnd程序主窗口
	bi.pidlRoot = NULL;
	bi.pszDisplayName = szBuffer;			// 返回选择的目录名的缓冲区
	bi.lpszTitle = _T("选择目录");		// 弹出的窗口的文字提示
	bi.ulFlags = BIF_RETURNONLYFSDIRS;	// 只返回目录。其他标志看MSDN
	bi.lpfn = NULL;                     // 回调函数，有时很有用
	bi.lParam = 0;
	bi.iImage = 0;
	ITEMIDLIST * pidl = ::SHBrowseForFolder(&bi);   // 显示弹出窗口，ITEMIDLIST很重要
	if (::SHGetPathFromIDList(pidl, szFullPath))    // 在ITEMIDLIST中得到目录名的整个路径
	{
		// 成功
		CEditUI* pOutPath = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("edt_pdf_out_path")));
		if (NULL != pOutPath)
		{
			pOutPath->SetText(CString(szFullPath));
		}
	}
	else
	{
		// 失败
	}

}

LRESULT CMainFrame::OnDropFiles(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HDROP hDropInfo = (HDROP)wParam;
	INT	nFileCount = ::DragQueryFile(hDropInfo, (UINT)-1, NULL, 0);
	if (nFileCount <= 0)
	{
		MessageBox(NULL, _T("请托一个PDF文件放到改区域！"), _T("提示"),MB_OK);
		return S_OK;
	}

	POINT point;
	::GetCursorPos(&point);
	ScreenToClient(m_hWnd, &point);
	CControlUI* pControlUI = static_cast<CControlUI*>(m_PaintManager.FindControl(point));
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
	
	//添加到list
	AddListItem(vecFileList);
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



CListContainerElementUI* CMainFrame::GetListItem(ST_LISTITEM_INFO item)
{

	static UINT32 uId = 0;

	CListContainerElementUI* pItem = new CListContainerElementUI;
	pItem->SetFixedHeight(40);
	pItem->SetBottomBorderSize(2);
	pItem->SetBorderColor(0xFFe6e6e6);

	CControlUI* pNode = new CControlUI();
	if (pNode){
		pNode->SetText(item.strPDFPath);
		pNode->SetTag(uId++);
	}
	pItem->SetTag((UINT_PTR)pNode);


	CHorizontalLayoutUI* pHorFileName = new CHorizontalLayoutUI;
	if (pHorFileName != NULL)
	{
		CLabelUI* pLableFileName = new CLabelUI;
		if (pLableFileName != NULL)
		{
			pLableFileName->SetTextPadding(CDuiRect(4, 0, 0, 0));
			pLableFileName->SetText(item.strPDFName);
			pLableFileName->SetTextStyle(DT_LEFT | DT_SINGLELINE | DT_VCENTER);
			pLableFileName->SetAttribute(_T("endellipsis"), _T("true"));
			pHorFileName->Add(pLableFileName);
		}
		pHorFileName->SetFixedWidth(150);
		pItem->Add(pHorFileName);
	}

	CHorizontalLayoutUI* pHorLine1 = new CHorizontalLayoutUI;
	if (pHorLine1 != NULL)
	{
		pHorLine1->SetFixedWidth(1);
		pItem->Add(pHorLine1);
	}

	CHorizontalLayoutUI* pHorFileSize = new CHorizontalLayoutUI;
	if (pHorFileSize != NULL)
	{
		CLabelUI* pLableFileSize = new CLabelUI;
		if (pLableFileSize != NULL)
		{
			pLableFileSize->SetTextPadding(CDuiRect(4, 0, 0, 0));
			pLableFileSize->SetText(item.strFileSize);
			pLableFileSize->SetTextStyle(DT_LEFT | DT_SINGLELINE | DT_VCENTER);
			pLableFileSize->SetAttribute(_T("endellipsis"), _T("true"));
			pHorFileSize->Add(pLableFileSize);
		}
		pHorFileSize->SetFixedWidth(150);
		pItem->Add(pHorFileSize);
	}

	CHorizontalLayoutUI* pHorLine2 = new CHorizontalLayoutUI;
	if (pHorLine2 != NULL)
	{
		pHorLine2->SetFixedWidth(1);
		pItem->Add(pHorLine2);
	}

	CHorizontalLayoutUI* pHorFileState = new CHorizontalLayoutUI;
	if (pHorFileState != NULL)
	{
		CLabelUI* pLableFileState = new CLabelUI;
	
		if (pLableFileState != NULL)
		{
			pLableFileState->SetTextPadding(CDuiRect(4, 0, 0, 0));
			pLableFileState->SetName(_T("lab_state"));
			pLableFileState->SetText(item.strState);
			pLableFileState->SetTextStyle(DT_LEFT | DT_SINGLELINE | DT_VCENTER);
			pLableFileState->SetAttribute(_T("endellipsis"), _T("true"));
			pHorFileState->Add(pLableFileState);
		}
		pHorFileState->SetFixedWidth(150);
		pItem->Add(pHorFileState);
	}



	return pItem;
}
