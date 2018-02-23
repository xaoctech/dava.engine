#include "VisualScriptEditor/Private/Models/TypeConversion.h"

#include <Base/Type.h>
#include <Reflection/ReflectedType.h>
#include <Reflection/ReflectedTypeDB.h>
#include <VisualScript/VisualScriptPin.h>

namespace DAVA
{
QtNodes::NodeDataType DAVATypeToNodeType(const VisualScriptPin* pin)
{
    QString id = "invalidPin";
    QString name = "invalidName";
    QString prettyName;
    bool hasData = false;

    if (pin != nullptr)
    {
        name = QString::fromLatin1(pin->GetName().c_str());
        hasData = pin->IsDataPin();
        if (hasData == true)
        {
            const Type* userType = pin->GetType();

            if (userType && userType->IsPointer())
            {
                userType = userType->Deref();
            }
            userType = userType->Decay();

            if (userType != nullptr && userType->GetName() != nullptr)
            {
                id = QString::fromLatin1(userType->GetName());

                const ReflectedType* refType = ReflectedTypeDB::GetByType(userType);
                if (refType != nullptr)
                {
                    prettyName = QString::fromStdString(refType->GetPermanentName());
                }
                else
                {
                    prettyName = QString::fromStdString(userType->GetDemangledName());
                }
            }
        }
        else
        {
            id = "Execution";
        }
    }

    return { id, name, prettyName, hasData, pin };
}

} // DAVA
