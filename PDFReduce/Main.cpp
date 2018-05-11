#include <atlstr.h>
#include "UIlib.h"
#include "UtilPath.h"
#include "MainFrame.h"
using namespace DuiLib;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int nCmdShow)
{
	
	// COM
	HRESULT Hr = ::CoInitialize(NULL);
	if (FAILED(Hr)) return 0;
	//设置资源管理
	CPaintManagerUI::SetInstance(hInstance);
	CString s = Util::Path::GetResFolder();
	CPaintManagerUI::SetResourcePath(s);

	//创建窗口
	CMainFrame* pMainFrame = new CMainFrame();
	if (pMainFrame == NULL)
		return 0;
	pMainFrame->Create(NULL, _T("悦书PDF工具"), UI_WNDSTYLE_FRAME, 0L, 0, 0, 0, 0);
	pMainFrame->CenterWindow();

	//消息循环
	CPaintManagerUI::MessageLoop();

	delete pMainFrame;
	pMainFrame = NULL;
	::CoUninitialize();
	return 0;
}
