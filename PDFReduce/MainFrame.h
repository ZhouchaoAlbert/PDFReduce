#pragma once
#include "UIlib.h"
using namespace  DuiLib;
#include <string>
using namespace std;

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
	
	//����PDFѹ��
	void StartPDFCompress();

   //ѡ���PDF·��
	void SelectPDFFolderDialog();
private:
	CTabLayoutUI* m_pTreeList;

};
