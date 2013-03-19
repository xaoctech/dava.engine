#ifndef __WEBVIEWCONTROL_WIN32_H__
#define __WEBVIEWCONTROL_WIN32_H__

#include <MsHTML.h>

#include "../../UI/IWebViewControl.h"
using namespace DAVA;

// Helper class to contain Web Browser.
interface IWebBrowser2;
class WebBrowserContainer : IOleClientSite, IOleInPlaceSite
{
public:
	// Construction/destruction.
	WebBrowserContainer();
	virtual ~WebBrowserContainer();

	// Initialize the browser on the parent window.
	bool Initialize(HWND parentWindow);

	// Update the rect according to the parent window.
	void UpdateRect();

	// Open the URL.
	bool OpenUrl(const WCHAR* urlToOpen);

	// COM stuff;
	HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject);

	ULONG __stdcall AddRef() { return 1; /*one instance*/ }
	ULONG __stdcall Release() { return 1; /*one instance*/ }

	// IOleWindow interface.
	HRESULT __stdcall GetWindow(HWND *phwnd);
	HRESULT __stdcall ContextSensitiveHelp(BOOL) { return S_OK; }

	// IOleInPlaceSite interface.
	HRESULT __stdcall CanInPlaceActivate() { return S_OK; }
	HRESULT __stdcall OnInPlaceActivate() { return S_OK; }
	HRESULT __stdcall OnUIActivate() { return S_OK; }
	HRESULT __stdcall GetWindowContext(IOleInPlaceFrame **ppFrame, IOleInPlaceUIWindow **ppDoc,
		LPRECT lprcPosRect, LPRECT lprcClipRect, LPOLEINPLACEFRAMEINFO lpFrameInfo);

	HRESULT __stdcall Scroll(SIZE) { return S_OK; }
	HRESULT __stdcall OnUIDeactivate(BOOL) { return S_OK; }
	HRESULT __stdcall OnInPlaceDeactivate() { return S_OK; }
	HRESULT __stdcall DiscardUndoState() { return S_OK; }
	HRESULT __stdcall DeactivateAndUndo() { return S_OK; }
	HRESULT __stdcall OnPosRectChange(LPCRECT) { return S_OK; }

	// IOleClientSite
	HRESULT __stdcall SaveObject() { return S_OK; }
	HRESULT __stdcall GetMoniker(DWORD, DWORD, IMoniker **ppmk) { return *ppmk = NULL, E_NOTIMPL; }
	HRESULT __stdcall GetContainer(IOleContainer** ppContainer) { return *ppContainer=NULL, E_NOINTERFACE; }
	HRESULT __stdcall ShowObject() { return S_OK; }
	HRESULT __stdcall OnShowWindow(BOOL) { return S_OK; }
	HRESULT __stdcall RequestNewObjectLayout() { return E_NOTIMPL; }

protected:
	// Parent window.
	HWND hwnd;

	// The browser itselt.
	IWebBrowser2* webBrowser;
};

// Web View Control for Win32.
class WebViewControl : public IWebViewControl
{
public:
	WebViewControl();
	virtual ~WebViewControl();
	
	// Initialize the control.
	virtual void Initialize(const Rect& rect);
	
	// Open the URL requested.
	virtual void OpenURL(const String& urlToOpen);
	
	// Size/pos/visibility changes.
	virtual void SetRect(const Rect& rect);
	virtual void SetVisible(bool isVisible, bool hierarchic);

protected:
	// Initialize the COM and create the browser container.
	bool InititalizeBrowserContainer();

	// Holder window for WebBrowser.
	HWND browserWindow;

	// Web Browser Container.
	WebBrowserContainer* browserContainer;
};

#endif //__WEBVIEWCONTROL_WIN32_H__
