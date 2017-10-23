#pragma once

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Math/Vector.h"
#include "Functional/Function.h"

namespace DAVA
{
class UIControl;

class UICommandMap final
{
public:
    struct CommandParams
    {
        FastName targetPath;
        String argument;
    };

    typedef DAVA::Function<void(UIControl*, const CommandParams&)> CommandFunction;

    UICommandMap() = default;
    ~UICommandMap() = default;

    void Put(const FastName& name, const CommandFunction& function);
    void Remove(const FastName& name);
    bool Perform(const FastName& name, UIControl* source, const CommandParams& params);

private:
    UnorderedMap<FastName, CommandFunction> functions;
};
}
