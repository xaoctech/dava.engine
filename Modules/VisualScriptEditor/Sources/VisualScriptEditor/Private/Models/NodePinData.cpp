#include "VisualScriptEditor/Private/Models/NodePinData.h"
#include "VisualScriptEditor/Private/Models/TypeConversion.h"

#include <nodes/NodeData>

#include <TArc/Qt/QtString.h>

#include <Base/Type.h>
#include <Debug/DVAssert.h>
#include <VisualScript/VisualScriptPin.h>

namespace DAVA
{
NodePinData::NodePinData(VisualScriptPin* pin_)
    : pin(pin_)
{
    DVASSERT(pin != nullptr);
}

QtNodes::NodeDataType NodePinData::type() const
{
    return DAVATypeToNodeType(pin);
}

VisualScriptPin* NodePinData::GetPin() const
{
    return pin;
}

} // DAVA
