#pragma once
#include "UIlib.h"
using namespace  DuiLib;
#include <string>
using namespace std;
#include "DropTargetLink.h"
#include "GSWin32Parse.h"
#include <atlstr.h>
using namespace ATL;

class CMainFrame : public WindowImplBase,
				   public IDropTargetLink
{
public:
	CMainFrame();
	~CMainFrame();

public:// UI初始化
	CDuiString GetSkinFolder();
	CDuiString GetSkinFile();
	LPCTSTR GetWindowClassName() const;
	CControlUI* CreateControl(LPCTSTR pstrClass);
	void InitWindow();
	void OnFinalMessage(HWND hWnd);
	LRESULT ResponseDefaultKeyEvent(WPARAM wParam);

	virtual void Notify(TNotifyUI& msg);

	LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnDropFiles(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	HRESULT OnDropEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
	HRESULT OnDropOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
	HRESULT OnDropLeave();
	HRESULT OnDrop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

	struct ST_LISTITEM_INFO
	{
		CString strPDFPath;
		CString strPDFName;
		CString strFileSize;
		CString strState;
	};
	CListContainerElementUI* GetListItem(ST_LISTITEM_INFO item);

	void AddListItem(vector<CString> vecFileList);
public:
	//启动PDF压缩
	void StartPDFCompress();
	//开始图片转换
	void StartPicConvert();
   //选择打开PDF路径
	void SelectPDFFolderDialog();
	void SelectOutDirectory();
private:
	CHorizontalLayoutUI* m_pHorFileList;
	CHorizontalLayoutUI* m_pHorFileDrop;
	CTabLayoutUI* m_pTreeList;
	CListUI*      m_pFileList;

	HICON         m_hIcon;
	UINT32        m_uCompressMode;
};

