#pragma once

#include "Debug/DebugOverlayItem.h"
#include "Concurrency/Atomic.h"

namespace DAVA
{
namespace DebugOverlayItemLoggerDetail
{
class LoggerOutputContainer;
}

class DebugOverlayItemLogger final : public DebugOverlayItem
{
public:
    DebugOverlayItemLogger();
    ~DebugOverlayItemLogger();

    String GetName() const override;
    void Draw(float32 elapsedTime) override;

private:
    std::unique_ptr<DebugOverlayItemLoggerDetail::LoggerOutputContainer> loggerOutput;
    Atomic<bool> collectingLogs{ false };

    friend class DebugOverlayItemLoggerDetail::LoggerOutputContainer;
};
}
