#pragma once

#include <Functional/Function.h>
#include <QObject>

namespace DAVA
{
namespace TArc
{
class QtDelayedExecutor : public QObject
{
public:
    QtDelayedExecutor(QObject* parent = nullptr);

    void DelayedExecute(const DAVA::Function<void()>& functor);

protected:
    bool event(QEvent* e) override;
};
} // namespace TArc
} // namespace DAVA
