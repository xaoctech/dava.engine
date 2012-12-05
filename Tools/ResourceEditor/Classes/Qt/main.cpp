#include <QtGui/QApplication>
#include "Main/mainwindow.h"
#include "DAVAEngine.h"

int main(int argc, char *argv[])
{
	int ret = 0;
    QApplication a(argc, argv);

#if defined (__DAVAENGINE_MACOS__)
    DAVA::Core::Run(argc, argv);
#elif defined (__DAVAENGINE_WIN32__)
	HINSTANCE hInstance = (HINSTANCE)::GetModuleHandle(NULL);
	DAVA::Core::Run(argc, argv, hInstance);
#else
	DVASSERT(false && "Wrong platform")
#endif

    new QtMainWindow();
    QtMainWindow::Instance()->show();

	ret = a.exec();

	QtMainWindow::Instance()->Release();

	return ret;
}
