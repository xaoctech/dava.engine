#include "TArc/Utils/QtThread.h"

namespace DAVA
{
namespace TArc
{
QtThread::QtThread(QObject* parent /*= nullptr*/)
    : QThread(parent)
{
}

#if defined(__DAVAENGINE_WINDOWS__)
void QtThread::run()
{
    QThread::exec();
}
#endif
} // namespace TArc
} // namespace DAVA
