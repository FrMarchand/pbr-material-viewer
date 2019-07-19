#ifndef DROPTARGET_H 
#define DROPTARGET_H
#include <ole2.h> 

#define WM_OLEDROP WM_USER + 1 

using call_back = void(*)(const char* path);

class DropTarget : public IDropTarget
{
private:
	unsigned long mRefs;
	bool mAcceptFormat;
	call_back mCallBack;

	// helper function 
	void __fastcall HandleDrop(HDROP HDrop);

protected:
	// IUnknown methods 
	STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();

	// IDropTarget methods 
	STDMETHOD(DragEnter)(LPDATAOBJECT pDataObj, DWORD grfKeyState,
		POINTL pt, LPDWORD pdwEffect);
	STDMETHOD(DragOver)(DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect);
	STDMETHOD(DragLeave)();
	STDMETHOD(Drop)(LPDATAOBJECT pDataObj, DWORD grfKeyState,
		POINTL pt, LPDWORD pdwEffect);

public:
	DropTarget(call_back cb);
	~DropTarget();
	bool AcceptFormat() { return mAcceptFormat; }

};
//--------------------------------------------------------------------------- 
#endif//DROPTARGET_H