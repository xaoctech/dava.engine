#include "UI/Events/UICommandMap.h"
#include "UI/UIControl.h"

namespace DAVA
{
void UICommandMap::Put(const FastName& name, const CommandFunction& function)
{
    functions[name] = function;
}

void UICommandMap::Remove(const FastName& name)
{
    auto it = functions.find(name);
    if (it != functions.end())
    {
        functions.erase(it);
    }
}

bool UICommandMap::Perform(const FastName& name, UIControl* source, const CommandParams& params)
{
    auto it = functions.find(name);
    if (it != functions.end())
    {
        it->second(source, params);
        return true;
    }

    return false;
}
}
