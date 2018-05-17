
#ifndef __DROPTARGETREAL_H__
#define __DROPTARGETREAL_H__
#include <OleIdl.h>
#include <ShObjIdl.h>
#include "DropTargetLink.h"

const GUID FAR CLSID_DragDropHelper = { 0x4657278a, 0x411b, 0x11d2, { 0x83, 0x9a, 0x0, 0xc0, 0x4f, 0xd9, 0x18, 0xd0 } };


class CDropTargetReal : public IDropTarget
{
public:
	CDropTargetReal();
	virtual ~CDropTargetReal();

	BOOL RegisterDropTarget(HWND hWnd, IDropTargetLink *pDuiDropTarget);
	void RevokeDropTarget(HWND hWnd);
public:
	//IUnkown
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);
	virtual ULONG STDMETHODCALLTYPE AddRef(void);
	virtual ULONG STDMETHODCALLTYPE Release(void);

	//IDropTarget
	virtual HRESULT STDMETHODCALLTYPE DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
	virtual HRESULT STDMETHODCALLTYPE DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
	virtual HRESULT STDMETHODCALLTYPE DragLeave(void);
	virtual HRESULT STDMETHODCALLTYPE Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

private:
	HWND				m_hWnd;				
	volatile LONG       m_lRef;
	IDropTargetHelper  *m_pDropTargetHelper;
	IDropTargetLink	   *m_pDropTargetLink;
};
#endif