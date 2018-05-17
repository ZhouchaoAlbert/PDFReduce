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

public:// UI��ʼ��
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

public:
	//����PDFѹ��
	void StartPDFCompress();
	//��ʼͼƬת��
	void StartPicConvert();
   //ѡ���PDF·��
	void SelectPDFFolderDialog();
	
private:
	CTabLayoutUI* m_pTreeList;
	HICON         m_hIcon;
	UINT32        m_uCompressMode;
};

