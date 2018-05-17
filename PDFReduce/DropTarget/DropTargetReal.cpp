#include "StdAfx.h"
#include "DropTargetReal.h"
#include <atlstr.h>

CDropTargetReal::CDropTargetReal()
:m_lRef(0),
m_hWnd(NULL),
m_pDropTargetLink(NULL)
{
	::CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER, IID_IDropTargetHelper, (void**)&m_pDropTargetHelper);
	ATLASSERT(m_pDropTargetHelper);
}
CDropTargetReal::~CDropTargetReal()
{
}
BOOL CDropTargetReal::RegisterDropTarget(HWND hWnd, IDropTargetLink *pDuiDropTarget)
{
	m_hWnd = hWnd;
	m_pDropTargetLink = pDuiDropTarget;
	::RegisterDragDrop(m_hWnd, this);
	return TRUE;
}
void CDropTargetReal::RevokeDropTarget(HWND hWnd)
{
	::RevokeDragDrop(m_hWnd);
}

HRESULT CDropTargetReal::QueryInterface(REFIID riid, void **ppvObject)
{
	HRESULT hr = E_NOINTERFACE;
	if (riid == __uuidof(IUnknown))
	{
		*ppvObject = (IUnknown*) this;
		hr = S_OK;
	}
	else if (riid == __uuidof(IDropTarget))
	{
		*ppvObject = (IDropTarget*)this;
		hr = S_OK;
	}
	if (SUCCEEDED(hr))
		AddRef();
	return hr;
}
ULONG CDropTargetReal::AddRef(void)
{
	return ++m_lRef;
}
ULONG CDropTargetReal::Release(void)
{
	m_lRef--;
	if (0 == m_lRef)
	{
		delete this;
	}
	ATLASSERT(0 <= m_lRef);
	return m_lRef;
}

HRESULT CDropTargetReal::DragEnter(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	if (NULL == pDataObj || NULL == m_hWnd)
	{
		return E_INVALIDARG;
	}

  	POINT ppt = { pt.x, pt.y };

	if (NULL != m_pDropTargetHelper)
	{
		m_pDropTargetHelper->DragEnter(m_hWnd, pDataObj, &ppt, *pdwEffect);
	}
	return m_pDropTargetLink->OnDropEnter(pDataObj, grfKeyState, pt, pdwEffect);
}
HRESULT CDropTargetReal::DragOver(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	POINT ppt = { pt.x, pt.y };

	if (NULL != m_pDropTargetHelper)
	{
		m_pDropTargetHelper->DragOver(&ppt, *pdwEffect);
	}
	return m_pDropTargetLink->OnDropOver(grfKeyState, pt, pdwEffect);
}
HRESULT CDropTargetReal::DragLeave(void)
{
	if (NULL != m_pDropTargetHelper)
	{
		m_pDropTargetHelper->DragLeave();
	}
	return m_pDropTargetLink->OnDropLeave();
}
HRESULT CDropTargetReal::Drop(IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect)
{
	if (NULL == pDataObj)
	{
		return E_INVALIDARG;
	}
	
	POINT ppt = { pt.x, pt.y };
	if (NULL != m_pDropTargetHelper)
	{
		m_pDropTargetHelper->Drop(pDataObj, &ppt, *pdwEffect);
	}

	return m_pDropTargetLink->OnDrop(pDataObj, grfKeyState, pt, pdwEffect);
}