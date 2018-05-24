#pragma once
#include "UIlib.h"
using namespace  DuiLib;
#include <string>
using namespace std;

#include <atlstr.h>
using namespace ATL;

#include "PDFCompress/PdfCompressEx.h"

class CMainFrame : public WindowImplBase
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


	CListContainerElementUI* GetListItem(ST_LISTITEM_INFO item);

	void AddListItem(vector<CString> vecFileList);
public:
	//����PDFѹ��
	void StartPDFCompress();
	//��ʼͼƬת��
	void StartPicConvert();
   //ѡ���PDF·��
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

