#ifndef __DROPTARGETLINK_H__
#define __DROPTARGETLINK_H__
#include <OleIdl.h>

class IDropTargetLink
{
public:
	virtual HRESULT OnDropEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect) = 0;
	virtual HRESULT OnDropOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect) = 0;
	virtual HRESULT OnDropLeave() = 0;
	virtual HRESULT OnDrop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect) = 0;
};

#endif