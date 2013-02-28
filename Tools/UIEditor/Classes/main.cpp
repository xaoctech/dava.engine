#include <QtGui/QApplication>
#include "mainwindow.h"

#include "DAVAEngine.h"

#if defined (__DAVAENGINE_MACOS__)
#include "Platform/Qt/MacOS/QtLayerMacOS.h"
#elif defined (__DAVAENGINE_WIN32__)
#include "Platform/Qt/Win32/QtLayerWin32.h"
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

#if defined (__DAVAENGINE_MACOS__)
    DAVA::Core::Run(argc, argv);
	new DAVA::QtLayerMacOS();
#elif defined (__DAVAENGINE_WIN32__)
	HINSTANCE hInstance = (HINSTANCE)::GetModuleHandle(NULL);
	DAVA::Core::Run(argc, argv, hInstance);
	new DAVA::QtLayerWin32();
#else
	DVASSERT(false && "Wrong platform")
#endif

    MainWindow w;
    w.show();

    return a.exec();
}
