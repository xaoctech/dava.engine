#include "QECommands/InsertControlCommand.h"

#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/PackageHierarchy/PackageControlsNode.h"
#include "Model/PackageHierarchy/PackageNode.h"

#include <UI/UIControl.h>
#include <Logger/Logger.h>


namespace InsertControlCommandDetails
{
using namespace DAVA;

bool IsNameExists(const String& name, ControlsContainerNode* dest, ControlsContainerNode* prototypes)
{
    auto& isEqual = [&name](ControlNode* siblingControl)
    {
        return (siblingControl->GetName() == name);
    };

    return std::any_of(dest->begin(), dest->end(), isEqual) || (prototypes != nullptr && std::any_of(prototypes->begin(), prototypes->end(), isEqual));
}

void SplitName(const String& name, String& nameMainPart, uint32& namePostfix)
{
    size_t underlinePos = name.rfind('_');
    if (underlinePos != String::npos && (underlinePos + 1) < name.size())
    {
        char anySymbol; // dummy symbol indicating that there are some symbols after %u in name string
        int res = sscanf(name.data() + underlinePos + 1, "%u%c", &namePostfix, &anySymbol);
        if (res == 1)
        {
            nameMainPart = name.substr(0, underlinePos);
            return;
        }
    }

    nameMainPart = name;
    namePostfix = 0;
}

String ProduceUniqueName(const String& name, ControlsContainerNode* dest, ControlsContainerNode* prototypes)
{
    String nameMainPart;
    uint32 namePostfixCounter = 0;
    SplitName(name, nameMainPart, namePostfixCounter);

    for (++namePostfixCounter; namePostfixCounter <= UINT32_MAX; ++namePostfixCounter)
    {
        String newName = Format("%s_%u", nameMainPart.c_str(), namePostfixCounter);
        if (!IsNameExists(newName, dest, prototypes))
        {
            return newName;
        }
    }

    DAVA::Logger::Warning("Can't produce unique name: reaching uint32 max");
    return name;
}
}

InsertControlCommand::InsertControlCommand(PackageNode* package, ControlNode* node_, ControlsContainerNode* dest_, int index_)
    : QEPackageCommand(package, "Insert Control")
    , node(DAVA::RefPtr<ControlNode>::ConstructWithRetain(node_))
    , dest(DAVA::RefPtr<ControlsContainerNode>::ConstructWithRetain(dest_))
    , index(index_)
{    
    ControlsContainerNode* prototypes = nullptr;
    if (dest->GetParent() == package)
    {
        // prototypes names scope is also taking into account if dest is a top level node
        prototypes = static_cast<ControlsContainerNode*>(package->GetPrototypes());
    }

    origName = node->GetName();
    if (InsertControlCommandDetails::IsNameExists(origName, dest.Get(), prototypes))
    {
        newName = InsertControlCommandDetails::ProduceUniqueName(origName, dest.Get(), prototypes);
    }
    else
    {
        newName = origName;
    }
}

void InsertControlCommand::Redo()
{
    node->GetControl()->SetName(newName);
    package->InsertControl(node.Get(), dest.Get(), index);
}

void InsertControlCommand::Undo()
{
    node->GetControl()->SetName(origName);
    package->RemoveControl(node.Get(), dest.Get());
}
