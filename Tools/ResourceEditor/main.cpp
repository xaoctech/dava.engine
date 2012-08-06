#include <QtGui/QApplication>
#include "mainwindow.h"

#include "DAVAEngine.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

#if defined (__DAVAENGINE_MACOS__)
    DAVA::Core::Run(argc, argv);
#elif defined (__DAVAENGINE_WIN32__)
	HINSTANCE hInstance = (HINSTANCE)::GetModuleHandle(NULL);
	DAVA::Core::Run(argc, argv, hInstance);
#else
	DVASSERT(false && "Wrong platform")
#endif

    QtMainWindow w;
    w.show();

    return a.exec();
}
