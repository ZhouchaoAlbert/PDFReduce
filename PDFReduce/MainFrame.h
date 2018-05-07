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
	
	//启动PDF压缩
	void StartPDFCompress();
	//开始图片转换
	void StartPicConvert();
   //选择打开PDF路径
	void SelectPDFFolderDialog();
private:
	CTabLayoutUI* m_pTreeList;

};

