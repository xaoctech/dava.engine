#pragma once

#include <Base/BaseTypes.h>

class ControlNode;

namespace LibraryDefaultControls
{
DAVA::Vector<DAVA::RefPtr<ControlNode>> CreateDefaultControls();
}