#pragma once

#include "Base/BaseTypes.h"

#include "Engine/Dispatcher.h"
#include "Engine/Private/Dispatcher/UIDispatcherEvent.h"

namespace DAVA
{
namespace Private
{
using UIDispatcher = Dispatcher<UIDispatcherEvent>;

} // namespace Private
} // namespace DAVA
