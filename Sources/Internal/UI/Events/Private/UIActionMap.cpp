#include "UI/Events/UIActionMap.h"

namespace DAVA
{
void UIActionMap::Put(const FastName& name, const SimpleAction& action)
{
    actions[name] = action;
}

void UIActionMap::Remove(const FastName& name)
{
    auto it = actions.find(name);
    if (it != actions.end())
    {
        actions.erase(it);
    }
}

bool UIActionMap::Perform(const FastName& name)
{
    auto it = actions.find(name);
    if (it != actions.end())
    {
        it->second();
        return true;
    }

    return false;
}
}
