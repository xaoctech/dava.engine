#pragma once

#ifndef __DAVA_Reflection__
#include "Reflection/Reflection.h"
#endif

#include "Reflection/Private/Wrappers/StructureWrapperDefault.h"
#include "Reflection/Private/Wrappers/ValueWrapperDefault.h"

namespace DAVA
{
template <typename C>
class StructureWrapperStdIdxStatic : public StructureWrapperDefault
{
public:
    using V = typename C::value_type;

    StructureWrapperStdIdxStatic(bool hasRangeAccess)
    {
        caps.canAddField = true;
        caps.canInsertField = false;
        caps.canRemoveField = false;
        caps.hasFlatStruct = true;
        caps.isContainer = true;
        caps.hasDynamicStruct = false;
        caps.hasRangeAccess = hasRangeAccess;
        caps.flatKeysType = Type::Instance<size_t>();
        caps.flatValuesType = Type::Instance<V>();
    }

    bool HasFields(const ReflectedObject& object, const ValueWrapper* vw) const override
    {
        C* c = vw->GetValueObject(object).GetPtr<C>();
        return (c->size() > 0);
    }

    size_t GetFieldsCount(const ReflectedObject& object, const ValueWrapper* vw) const override
    {
        if (!caps.hasRangeAccess)
            return 0;

        C* c = vw->GetValueObject(object).GetPtr<C>();
        return c->size();
    }

    Reflection GetField(const ReflectedObject& obj, const ValueWrapper* vw, const Any& key) const override
    {
        if (key.CanCast<size_t>())
        {
            size_t i = key.Cast<size_t>();
            return GetField(obj, vw, i).ref;
        }

        return Reflection();
    }

    Reflection::Field GetField(const ReflectedObject& obj, const ValueWrapper* vw, size_t index) const override
    {
        C* c = vw->GetValueObject(obj).GetPtr<C>();
        auto it = std::next(c->begin(), index);
        if (it != c->end())
        {
            V* v = &(*it);
            return Reflection::Field(Any(index), Reflection::Create(v), nullptr);
        }

        return Reflection::Field();
    }

    Vector<Reflection::Field> GetFields(const ReflectedObject& obj, const ValueWrapper* vw, Reflection::MetaPredicate pred) const override
    {
        Vector<Reflection::Field> ret;

        C* c = vw->GetValueObject(obj).GetPtr<C>();

        size_t key = 0;
        ret.reserve(c->size());
        for (auto& it : *c)
        {
            if (nullptr == pred || (*pred)(nullptr))
            {
                V* v = &it;
                ret.emplace_back(Any(key++), Reflection::Create(v), nullptr);
            }
        }

        return ret;
    }

    Vector<Reflection::Field> GetFields(const ReflectedObject& obj, const ValueWrapper* vw, size_t first, size_t count) const override
    {
        Vector<Reflection::Field> ret;

        if (caps.hasRangeAccess)
        {
            C* c = vw->GetValueObject(obj).GetPtr<C>();
            size_t sz = c->size();

            DVASSERT(first < sz);
            DVASSERT(first + count <= sz);

            if (first < sz)
            {
                size_t n = std::min(count, sz - first);
                size_t i = first;

                ret.reserve(n);

                auto it = std::next(c->begin(), first);
                auto end = std::next(it, n);
                for (; it != end; ++it)
                {
                    V* v = &(*it);
                    ret.emplace_back(Any(i++), Reflection::Create(v), nullptr);
                }
            }
        }

        return ret;
    }

    bool AddField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key, const Any& value) const override
    {
        return false;
    }

    bool InsertField(const ReflectedObject& object, const ValueWrapper* vw, const Any& beforeKey, const Any& key, const Any& value) const override
    {
        return false;
    }

    bool RemoveField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const override
    {
        return false;
    }
};

template <typename C>
class StructureWrapperStdIdxDynamic : public StructureWrapperStdIdxStatic<C>
{
public:
    using V = typename C::value_type;

    StructureWrapperStdIdxDynamic(bool hasRangeAccess)
        : StructureWrapperStdIdxStatic<C>(hasRangeAccess)
    {
        StructureWrapperStdIdxStatic<C>::caps.hasDynamicStruct = true;
        StructureWrapperStdIdxStatic<C>::caps.canInsertField = true;
        StructureWrapperStdIdxStatic<C>::caps.canRemoveField = true;
    }

    bool AddField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key, const Any& value) const override
    {
        if (vw->IsReadonly(object))
        {
            return false;
        }

        C* c = vw->GetValueObject(object).GetPtr<C>();
        c->push_back(value.Get<V>());

        return true;
    }

    bool InsertField(const ReflectedObject& object, const ValueWrapper* vw, const Any& beforeKey, const Any& key, const Any& value) const override
    {
        if (vw->IsReadonly(object))
        {
            return false;
        }

        if (!beforeKey.CanCast<size_t>())
        {
            return false;
        }

        size_t i = beforeKey.Cast<size_t>();
        C* c = vw->GetValueObject(object).GetPtr<C>();

        auto it = std::next(c->begin(), i);
        c->insert(it, value.Get<V>());

        return true;
    }

    bool RemoveField(const ReflectedObject& object, const ValueWrapper* vw, const Any& key) const override
    {
        if (vw->IsReadonly(object))
        {
            return false;
        }

        if (!key.CanCast<size_t>())
        {
            return false;
        }

        size_t i = key.Cast<size_t>();
        C* c = vw->GetValueObject(object).GetPtr<C>();

        if (i >= c->size())
        {
            return false;
        }

        auto it = std::next(c->begin(), i);
        it = c->erase(it);

        return true;
    }
};

template <typename T, size_t N>
struct StructureWrapperCreator<Array<T, N>>
{
    static StructureWrapper* Create()
    {
        return new StructureWrapperStdIdxStatic<Array<T, N>>(true);
    }
};

template <typename T>
struct StructureWrapperCreator<Vector<T>>
{
    static StructureWrapper* Create()
    {
        return new StructureWrapperStdIdxDynamic<Vector<T>>(true);
    }
};

template <typename T>
struct StructureWrapperCreator<List<T>>
{
    static StructureWrapper* Create()
    {
        return new StructureWrapperStdIdxDynamic<List<T>>(false);
    }
};

} // namespace DAVA
