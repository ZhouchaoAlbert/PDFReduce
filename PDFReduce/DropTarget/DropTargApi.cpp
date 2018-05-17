#include "StdAfx.h"
#include "DropTargApi.h"
#include "DropTargetReal.h"
#include "Singleton.h"

using namespace ATL;

BOOL Util::DropTarget::Register(HWND hWnd, IDropTargetLink *pDropTargetLink)
{
	Singleton<CDropTargetReal>::Instance().RegisterDropTarget(hWnd, pDropTargetLink);
	return TRUE;
}

BOOL Util::DropTarget::UnRegister(HWND hWnd)
{
	Singleton<CDropTargetReal>::Instance().RevokeDropTarget(hWnd);
	return TRUE;
}