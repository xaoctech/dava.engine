#pragma once

#include "Classes/Selection/Selectable.h"

#include <TArc/Qt/QtString.h>

#include <Base/Any.h>
#include <Base/BaseTypes.h>
#include <Reflection/ReflectedTypeDB.h>

#include <QMimeData>
#include <QVariant>

class ReflectedMimeData : public QMimeData
{
public:
    ReflectedMimeData(const DAVA::Vector<Selectable>& objects_)
        : objects(objects_)
    {
#if defined(__DAVAENGINE_DEBUG__)
        for (const Selectable& obj : objects)
        {
            const DAVA::ReflectedType* type = obj.GetObjectType();
            DVASSERT(type != nullptr);
            DVASSERT(type->GetPermanentName().empty() == false);
        }
#endif
    }

    template <typename T>
    DAVA::Vector<T*> GetObjects() const
    {
        DAVA::Vector<T*> result;
        result.reserve(objects.size());
        for (const Selectable& obj : objects)
        {
            if (obj.CanBeCastedTo<T>())
            {
                result.push_back(obj.Cast<T>());
            }
        }

        return result;
    }

    template <typename T>
    bool HasObjects() const
    {
        for (const Selectable& obj : objects)
        {
            if (obj.CanBeCastedTo<T>())
            {
                return true;
            }
        }

        return false;
    }

    template <typename T>
    static QString GetTypeName()
    {
        const DAVA::ReflectedType* type = DAVA::ReflectedTypeDB::Get<T>();
        const DAVA::String& name = type->GetPermanentName();
        DVASSERT(name.empty() == false);

        return QString::fromStdString(name);
    }

private:
    DAVA::Vector<Selectable> objects;
};
