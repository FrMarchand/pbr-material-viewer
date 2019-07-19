#include "droptarget.h"
#include <string>

DropTarget::DropTarget(call_back cb)
	: IDropTarget()
{
	mRefs = 1;
	mAcceptFormat = false;
	mCallBack = cb;
}

DropTarget::~DropTarget()
{
}

// helper routine to notify Form of drop on target 
void __fastcall DropTarget::HandleDrop(HDROP HDrop)
{
	if (mAcceptFormat) {
		TCHAR szFile[MAX_PATH];
		UINT cch = DragQueryFile(HDrop, 0, szFile, MAX_PATH);
		if (cch > 0 && cch < MAX_PATH) {
			mCallBack(szFile);
		}
	}
}

// IUnknown Interface has three member functions: 
// QueryInterface, AddRef, and Release.

STDMETHODIMP DropTarget::QueryInterface(REFIID iid, void FAR* FAR* ppv)
{
	// tell other objects about our capabilities 
	if (iid == IID_IUnknown || iid == IID_IDropTarget) {
		*ppv = this;
		AddRef();
		return NOERROR;
	}

	*ppv = NULL;
	return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) DropTarget::AddRef()
{
	return ++mRefs;
}

STDMETHODIMP_(ULONG) DropTarget::Release()
{
	if (--mRefs == 0)
	{
		delete this;
		return 0;
	}
	return mRefs;
}


// Drag Enter is called first 
STDMETHODIMP DropTarget::DragEnter(LPDATAOBJECT pDataObj, DWORD grfKeyState,
	POINTL pt, LPDWORD pdwEffect)
{
	FORMATETC fmtetc;

	fmtetc.cfFormat = CF_HDROP;
	fmtetc.ptd = NULL;
	fmtetc.dwAspect = DVASPECT_CONTENT;
	fmtetc.lindex = -1;
	fmtetc.tymed = TYMED_HGLOBAL;

	STGMEDIUM medium;
	HRESULT hr = pDataObj->GetData(&fmtetc, &medium);

	UINT fileCount = 0;

	if (!FAILED(hr)) {
		// grab a pointer to the data 
		HGLOBAL HFiles = medium.hGlobal;
		HDROP HDrop = (HDROP)GlobalLock(HFiles);
		fileCount = DragQueryFile(HDrop, 0xFFFFFFFF, NULL, 0);
		// release the pointer to the memory 
		GlobalUnlock(HFiles);
		ReleaseStgMedium(&medium);
	}

	// does the drag source provide CF_HDROP, 
	// which is the only format we accept 
	mAcceptFormat = (pDataObj->QueryGetData(&fmtetc) == NOERROR) && (fileCount == 1);

	return NOERROR;
}

// implement visual feedback if required 
STDMETHODIMP DropTarget::DragOver(DWORD grfKeyState, POINTL pt,
	LPDWORD pdwEffect)
{
	if (mAcceptFormat) {
		*pdwEffect = DROPEFFECT_COPY;
	}
	else {
		*pdwEffect = DROPEFFECT_NONE;
	}

	return NOERROR;
}

// remove visual feedback 
STDMETHODIMP DropTarget::DragLeave()
{
	mAcceptFormat = false;
	return NOERROR;
}

// source has sent the DRAGDROP_DROP message indicating 
// a drop has a occurred 
STDMETHODIMP DropTarget::Drop(LPDATAOBJECT pDataObj, DWORD grfKeyState,
	POINTL pt, LPDWORD pdwEffect)
{
	HRESULT result = NOERROR;
	FORMATETC fmtetc;
	fmtetc.cfFormat = CF_HDROP;
	fmtetc.ptd = NULL;
	fmtetc.dwAspect = DVASPECT_CONTENT;
	fmtetc.lindex = -1;
	fmtetc.tymed = TYMED_HGLOBAL;

	// user has dropped on us -- get the CF_HDROP data from drag source 
	STGMEDIUM medium;
	HRESULT hr = pDataObj->GetData(&fmtetc, &medium);

	if (!FAILED(hr)) {
		// grab a pointer to the data 
		HGLOBAL HFiles = medium.hGlobal;
		HDROP HDrop = (HDROP)GlobalLock(HFiles);

		// call the helper routine which will notify the Form 
		// of the drop 
		HandleDrop(HDrop);

		// release the pointer to the memory 
		GlobalUnlock(HFiles);
		ReleaseStgMedium(&medium);
	}
	else {
		*pdwEffect = DROPEFFECT_NONE;
		result = hr;
	}

	mAcceptFormat = false;
	return result;
}