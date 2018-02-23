#include "Reflection/ReflectionSerializator.h"
#include "Reflection/ReflectedTypeDB.h"
#include "Reflection/ReflectedType.h"
#include "Logger/Logger.h"

#if !defined(__DAVAENGINE_WINDOWS__)
#include <cxxabi.h>
#endif //#if !defined (__DAVAENGINE_WINDOWS__)
#include <iomanip>

namespace DAVA
{
using PrinterFn = void (*)(std::ostringstream&, const Any&);
using PrintersTable = Map<const Type*, PrinterFn>;

const PrintersTable valuePrinters =
{
  { Type::Instance<int32>(), [](std::ostringstream& out, const Any& any) { out << any.Get<int32>(); } },
  { Type::Instance<uint32>(), [](std::ostringstream& out, const Any& any) { out << any.Get<uint32>(); } },
  { Type::Instance<int64>(), [](std::ostringstream& out, const Any& any) { out << any.Get<int64>(); } },
  { Type::Instance<uint64>(), [](std::ostringstream& out, const Any& any) { out << any.Get<uint64>(); } },
  { Type::Instance<float32>(), [](std::ostringstream& out, const Any& any) { out << any.Get<float32>(); } },
  { Type::Instance<float64>(), [](std::ostringstream& out, const Any& any) { out << any.Get<float64>(); } },
  { Type::Instance<String>(), [](std::ostringstream& out, const Any& any) { out << any.Get<String>().c_str(); } },
  { Type::Instance<FastName>(), [](std::ostringstream& out, const Any& any) { out << any.Get<FastName>().c_str(); } },
  { Type::Instance<size_t>(), [](std::ostringstream& out, const Any& any) { out << any.Get<size_t>(); } },
  { Type::Instance<bool>(), [](std::ostringstream& out, const Any& any) { out << any.Get<bool>(); } },
  { Type::Instance<void>(), [](std::ostringstream& out, const Any& any) { out << "???"; } }
};

const PrinterFn pointerPrinter = [](std::ostringstream& out, const Any& any) { out << "0x" << std::setw(8) << std::setfill('0') << std::hex << any.Cast<void*>(); };

static PrinterFn GetPrinterFns(const Type* type)
{
    PrinterFn ret = [](std::ostringstream& out, const Any&)
    {
        out << "(Unknown Any Type)";
    };

    if (nullptr != type)
    {
        const PrintersTable* pt = &valuePrinters;
        const Type* keyType = type;
        if (type->IsPointer())
        {
            ret = pointerPrinter;
            return ret;
        }

        if (nullptr != type->Decay())
            type = type->Decay();

        auto it = pt->find(type);
        if (it != pt->end())
        {
            ret = it->second;
        }
    }
    return ret;
}

String DumpAny(const Any& any)
{
    std::ostringstream line;
    PrinterFn fns = GetPrinterFns(any.GetType());
    if (fns != nullptr)
    {
        (*fns)(line, any);
    }
    else
    {
        (*fns)(line, any);
    }
    return line.str();
}

String DumpType(const Type* type_)
{
    if (type_ == nullptr)
        return String("nullptr");

    const Type* type = type_;
    if (type_->IsPointer())
        type = type_->Deref();

    const ReflectedType* rType = ReflectedTypeDB::GetByType(type);
    if (!rType)
    {
        String result;
        const char* symbol = type->GetName();
#if !defined(__DAVAENGINE_WINDOWS__)
        char* demangled = abi::__cxa_demangle(symbol, nullptr, nullptr, nullptr);
        if (demangled != nullptr)
        {
            result = demangled;
            free(demangled);
        }
#else
        DVASSERT(false);
#endif //#if !defined (__DAVAENGINE_WINDOWS__)
        return result;
    }
    else
    {
        String result;
        const char* symbol = type->GetName();
#if !defined(__DAVAENGINE_WINDOWS__)
        char* demangled = abi::__cxa_demangle(symbol, nullptr, nullptr, nullptr);
        if (demangled != nullptr)
        {
            result = demangled;
            free(demangled);
        }
#else
        DVASSERT(false);
#endif //#if !defined (__DAVAENGINE_WINDOWS__)

        return String("p:") + rType->GetPermanentName() + String("t:") + result;
    }
}

ReflectionSerializatorContext::PointerID ReflectionSerializatorContext::RegisterPointer(const Any& pointer)
{
    auto it = sharedPointers.find(pointer);
    if (it != sharedPointers.end())
    {
        // get second from iterator, than take second from pair
        return it->second;
    }
    PointerID newPointerID = nextUniquePointerID++;
    sharedPointers.emplace(pointer, newPointerID);
    return newPointerID;
}

ReflectionSerializatorContext::PointerID ReflectionSerializatorContext::FindPointer(const Any& pointer)
{
    auto it = sharedPointers.find(pointer);
    if (it != sharedPointers.end())
    {
        // get second from iterator, than take second from pair
        return it->second;
    }
    return 0;
}

void ReflectionSerializator::Save(const Reflection& reflection, const FilePath& filepath)
{
    ReflectionSerializatorContext context;
    SaveRecursive(Any(FastName("this")), reflection, context, context.objects);

    // Dump
    for (auto sharedObj : context.sharedPointers)
    {
        const Any& any = sharedObj.first;
        const Type* type = any.GetType();
        const ReflectedType* rType = ReflectedTypeDB::GetByType(type);
        Logger::Debug("%s = ?", rType->GetPermanentName().c_str());
    }
    DumpHierarchy(context.objects);
}

void ReflectionSerializator::SaveRecursive(const Any& key, const Reflection& reflection, ReflectionSerializatorContext& context, UnorderedMap<Any, Any>& objects)
{
    DVASSERT(reflection.IsValid() == true);

    /*const Reflection::FieldCaps& caps = reflection.GetFieldsCaps();
    if (caps.hasFlatStruct)
    {
        objects.emplace(key, reflection.GetValue());
        return;
    }*/

    Any value = reflection.GetValue();
    const Type* type = value.GetType();

    //    if (type->IsPointer())
    //    {
    //        if (context.FindPointer(value) == 0)
    //        {
    //            ReflectionSerializatorContext::PointerID pointerID = context.RegisterPointer(value);
    //            ReflectionSerializatorContext::PointerRef ptrRef(pointerID);
    //            objects.emplace(key, Any(ptrRef));
    //        }else
    //            return;
    //    }

    if (reflection.HasFields())
    {
        UnorderedMap<Any, Any> childObjects;
        Vector<Reflection::Field> fields = reflection.GetFields();
        for (auto& field : fields)
        {
            if (field.ref.GetMeta<M::Serialize>())
                SaveRecursive(field.key, field.ref, context, childObjects);
        }
        objects.emplace(key, Any(childObjects));
    }
    else
    {
        // Should be data
        objects.emplace(key, value);
    }
}
void ReflectionSerializator::DumpHierarchy(const UnorderedMap<Any, Any>& objects, uint32 level)
{
    for (auto& item : objects)
    {
        const Any& key = item.first;
        const Any& value = item.second;

        Logger::Debug("%s %s = (%s)", GetIndentString('-', level).c_str(),
                      DumpAny(key).c_str(), DumpType(value.GetType()).c_str());

        if (value.CanGet<UnorderedMap<Any, Any>>())
        {
            const UnorderedMap<Any, Any>& childObjects = value.Get<UnorderedMap<Any, Any>>();
            DumpHierarchy(childObjects, level + 1);
        }
    }
}

void ReflectionSerializator::Load(const Reflection& reflection, const FilePath& filepath)
{
    ReflectionSerializatorContext context;
    // Load data to context
    LoadRecursive(Any("this"), reflection, context, context.objects);
}

void ReflectionSerializator::LoadRecursive(const Any& key, const Reflection& reflection, ReflectionSerializatorContext& context, const UnorderedMap<Any, Any>& objects)
{
    const Reflection::FieldCaps& caps = reflection.GetFieldsCaps();
    if (caps.hasFlatStruct)
    {
        auto it = objects.find(key);
        if (it != objects.end())
        {
            const Any& value = it->second;
            if (!value.IsEmpty())
                reflection.SetValue(value);
        }
        return;
    }

    Any value = reflection.GetValue();
    const Type* type = value.GetType();

    if (type->IsPointer())
    {
        /*ReflectionSerializatorContext::PointerID pointerID = context.RegisterPointer(value);
        ReflectionSerializatorContext::PointerRef ptrRef(pointerID);
        objects.emplace(key, Any(ptrRef));*/
        auto it = objects.find(key);
        if (it != objects.end())
        {
            const Any& value = it->second;
            ReflectionSerializatorContext::PointerRef ptrRef = value.Get<ReflectionSerializatorContext::PointerRef>();
            Any ptr = context.reverseSharedPointers[ptrRef.pointer];
            if (!ptr.IsEmpty())
                reflection.SetValue(ptr);
        }
        return;
    }

    if (reflection.HasFields())
    {
        Vector<Reflection::Field> fields = reflection.GetFields();
        for (auto& field : fields)
        {
            auto it = objects.find(field.key);
            if (it != objects.end())
            {
                Any childMapAny = it->second;
                if (!childMapAny.IsEmpty())
                {
                    const UnorderedMap<Any, Any>& childObjects = childMapAny.Get<UnorderedMap<Any, Any>>();
                    LoadRecursive(field.key, field.ref, context, childObjects);
                }
            }
        }
    }
    else
    {
        auto it = objects.find(key);
        if (it != objects.end())
        {
            const Any& value = it->second;
            reflection.SetValue(value);
        }
    }
}
};
