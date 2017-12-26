#pragma once

#include "Debug/DebugOverlayItem.h"

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
    void Draw() override;

private:
    std::unique_ptr<DebugOverlayItemLoggerDetail::LoggerOutputContainer> loggerOutput;
};
}