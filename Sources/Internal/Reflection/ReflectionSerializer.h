#pragma once
#if 0
#include <ostream>
#include <istream>
#include "Reflection/Reflection.h"

namespace DAVA
{
struct PublicSerializer
{
    virtual void Write(std::ostream& out, const Any& any) = 0;
    virtual void Read(std::istream& in, Any& any) = 0;
};

struct DefaultBinarySerializer : public PublicSerializer
{
    void Write(std::ostream& out, const Any& any) override;
    void Read(std::istream& in, Any& any) override;
};

class ReflectionSerializer final
{
public:
    static void Save(std::ostream& out, const Any& key, const Reflection& ref, void* context, bool (*filter)(void*, const Reflection::Field&) = nullptr, PublicSerializer* saver = nullptr);
};

} // namespace DAVA
#endif
