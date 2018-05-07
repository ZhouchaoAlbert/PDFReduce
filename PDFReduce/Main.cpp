#include <atlstr.h>
#include "UIlib.h"
#include "MainFrame.h"
using namespace DuiLib;


ATL::CString GetResFolder()
{
	TCHAR szPath[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szPath, MAX_PATH);
	CString strPath(szPath);
	INT32 iFind = strPath.ReverseFind(_T('\\'));
	if (iFind < 0)
	{
		return _T("");
	}
	strPath = strPath.Left(iFind);

	INT32 ix = strPath.ReverseFind(_T('\\'));
	if (ix != -1)
	{
		return strPath.Mid(0, ix) + _T("\\Resource");
	}
	return _T("");
}


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int nCmdShow)
{
	// COM
	HRESULT Hr = ::CoInitialize(NULL);
	if (FAILED(Hr)) return 0;
	//������Դ����
	CPaintManagerUI::SetInstance(hInstance);
	CString s = GetResFolder();
	CPaintManagerUI::SetResourcePath(s);

	//��������
	CMainFrame* pMainFrame = new CMainFrame();
	if (pMainFrame == NULL)
		return 0;
	pMainFrame->Create(NULL, _T("PDFѹ�����"), UI_WNDSTYLE_FRAME, 0L, 0, 0, 0, 0);
	pMainFrame->CenterWindow();

	//��Ϣѭ��
	CPaintManagerUI::MessageLoop();

	delete pMainFrame;
	pMainFrame = NULL;
	::CoUninitialize();
	return 0;
}
