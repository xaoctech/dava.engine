#include "VisualScriptEditor/Private/Models/VisualScriptRegistryModel.h"
#include "VisualScriptEditor/Private/Models/VisualScriptNodeModel.h"
#include "VisualScriptEditor/Private/VisualScriptEditorDialogSettings.h"

#include <TArc/Core/ContextAccessor.h>
#include <TArc/WindowSubSystem/UI.h>

#include <Base/Exception.h>
#include <Logger/Logger.h>
#include <Reflection/Reflection.h>
#include <Reflection/ReflectedType.h>
#include <Reflection/ReflectedTypeDB.h>
#include <Reflection/ReflectedStructure.h>
#include <VisualScript/Nodes/VisualScriptAnotherScriptNode.h>
#include <VisualScript/Nodes/VisualScriptEventNode.h>
#include <VisualScript/VisualScript.h>
#include <VisualScript/VisualScriptEvents.h>
#include <VisualScript/VisualScriptNode.h>

#include <locale>

namespace DAVA
{
namespace VisualScriptRegistryModelDetails
{
void EnumerateDerivedTypes(const Type* type, Vector<FastName>& typeNames)
{
    DVASSERT(type != nullptr);

    const TypeInheritance* inheritance = type->GetInheritance();
    Vector<TypeInheritance::Info> derivedTypes = inheritance->GetDerivedTypes();
    for (const TypeInheritance::Info& derived : derivedTypes)
    {
        const ReflectedType* refType = ReflectedTypeDB::GetByType(derived.type);
        if (refType == nullptr)
        {
            continue;
        }

        typeNames.emplace_back(refType->GetPermanentName());
        EnumerateDerivedTypes(derived.type, typeNames);
    }
}

} //VisualScriptRegistryModelDetails

VisualScriptRegistryModel::VisualScriptRegistryModel(ContextAccessor* accessor_, UI* ui_, VisualScript* script_)
    : QtNodes::DataModelRegistry()
    , accessor(accessor_)
    , ui(ui_)
    , script(script_)
{
}

VisualScriptRegistryModel::~VisualScriptRegistryModel()
{
    ReleaseReflectedItems(staticReflectedItems);
    ReleaseReflectedItems(dynamicReflectedItems);
}

void VisualScriptRegistryModel::SetDataContainers(const Vector<Reflection>& dataContainer_)
{
    dataContainer = dataContainer_;
}

const std::map<QString, std::vector<QtNodes::RegistryItemDescriptor*>>& VisualScriptRegistryModel::GetStaticModels() const
{
    return staticReflectedItems;
}

const std::map<QString, std::vector<QtNodes::RegistryItemDescriptor*>>& VisualScriptRegistryModel::GetDynamicModels()
{
    ReleaseReflectedItems(dynamicReflectedItems);
    RegisterReflectedData();
    RegisterSubscripts();

    return dynamicReflectedItems;
}

RegistryItemDescriptorData* VisualScriptRegistryModel::CreateStaticItem(const QString& category, const QString& name)
{
    return CreateItemInternal(category, name, true);
}

RegistryItemDescriptorData* VisualScriptRegistryModel::CreateDynamicItem(const QString& category, const QString& name)
{
    return CreateItemInternal(category, name, false);
}

RegistryItemDescriptorData* VisualScriptRegistryModel::CreateItemInternal(const QString& category, const QString& name, bool staticItem)
{
    QtNodes::RegistryItemDescriptor* descriptor = new QtNodes::RegistryItemDescriptor();
    RegistryItemDescriptorData* descrData = new RegistryItemDescriptorData();
    descriptor->userData = descrData;

    descriptor->category = category;
    descriptor->name = name;

    if (staticItem)
    {
        staticReflectedItems[descriptor->category].push_back(descriptor);
    }
    else
    {
        dynamicReflectedItems[descriptor->category].push_back(descriptor);
    }

    return descrData;
}

void VisualScriptRegistryModel::RegisterEvents()
{
    Vector<FastName> eventNames;
    VisualScriptRegistryModelDetails::EnumerateDerivedTypes(Type::Instance<VisualScriptEvent>(), eventNames);

    for (const FastName& eName : eventNames)
    {
        RegistryItemDescriptorData* descrData = CreateStaticItem("Event", eName.c_str());
        descrData->creator = [this, eName]() {
            const ReflectedType* scriptNodeType = ReflectedTypeDB::GetByPermanentName(eName.c_str());
            VisualScriptEventNode* node = script->CreateNode<VisualScriptEventNode>();
            node->SetEventName(eName);
            return node;
        };
    }
}

void VisualScriptRegistryModel::RegisterControllers()
{
    { // Add branch
        RegistryItemDescriptorData* descrData = CreateStaticItem("Branch", "Branch");
        descrData->creator = [this]() {
            const ReflectedType* scriptNodeType = ReflectedTypeDB::GetByPermanentName("VisualScriptBranchNode");
            return script->CreateNode(scriptNodeType);
        };
    }

    { // add loops
        QString category = "Loops";

        { // for
            RegistryItemDescriptorData* descrData = CreateStaticItem("Loops", "For");
            descrData->creator = [this]() {
                const ReflectedType* scriptNodeType = ReflectedTypeDB::GetByPermanentName("VisualScriptForNode");
                return script->CreateNode(scriptNodeType);
            };
        }

        { // while
            RegistryItemDescriptorData* descrData = CreateStaticItem("Loops", "While");

            descrData->creator = [this]() {
                const ReflectedType* scriptNodeType = ReflectedTypeDB::GetByPermanentName("VisualScriptWhileNode");
                return script->CreateNode(scriptNodeType);
            };
        }
    }
}

void VisualScriptRegistryModel::RegisterReflectedMethods()
{
    for (const auto& pair : ReflectedTypeDB::GetPermamentNamesWithTypes())
    {
        const String& permanentName = pair.first;
        const ReflectedType* type = pair.second;
        DVASSERT(type != nullptr);

        const ReflectedStructure* structure = type->GetStructure();
        if (structure != nullptr)
        {
            for (const std::unique_ptr<ReflectedStructure::Field>& f : structure->fields)
            {
                String title = f->name.c_str();
                title[0] = std::toupper(title[0], std::locale()); // Capitalize first letter

                {
                    RegistryItemDescriptorData* descrData = CreateStaticItem(QString::fromStdString(permanentName), QString::fromLatin1(("Get" + title).c_str()));

                    FastName arg1 = FastName(permanentName.c_str());
                    FastName arg2 = f->name;
                    descrData->creator = [arg1, arg2, this]() {
                        const ReflectedType* scriptNodeType = ReflectedTypeDB::GetByPermanentName("VisualScriptGetMemberNode");
                        return script->CreateNode(scriptNodeType, arg1, arg2);
                    };
                }

                if (f->meta == nullptr || f->meta->GetMeta<M::ReadOnly>() == nullptr)
                {
                    RegistryItemDescriptorData* descrData = CreateStaticItem(QString::fromStdString(permanentName), QString::fromLatin1(("Set" + title).c_str()));

                    FastName arg1 = FastName(permanentName.c_str());
                    FastName arg2 = f->name;
                    descrData->creator = [arg1, arg2, this]() {
                        const ReflectedType* scriptNodeType = ReflectedTypeDB::GetByPermanentName("VisualScriptSetMemberNode");
                        return script->CreateNode(scriptNodeType, arg1, arg2);
                    };
                }
            }

            for (const std::unique_ptr<ReflectedStructure::Method>& m : structure->methods)
            {
                RegistryItemDescriptorData* descrData = CreateStaticItem(QString::fromStdString(permanentName), QString::fromLatin1(m->name.c_str()));

                FastName arg1 = FastName(permanentName.c_str());
                FastName arg2 = m->name;
                descrData->creator = [arg1, arg2, this]() {
                    const ReflectedType* scriptNodeType = ReflectedTypeDB::GetByPermanentName("VisualScriptFunctionNode");
                    return script->CreateNode(scriptNodeType, arg1, arg2);
                };
            }
        }
    }
}

void VisualScriptRegistryModel::RegisterSubscriptsAction()
{
    RegistryItemDescriptorData* descrData = CreateStaticItem("Another Script: Load", "Select *.dvs");
    descrData->creator = [this]()
    {
        using namespace DAVA;

        FileDialogParams fParams;
        fParams.title = "Select Script File...";
        fParams.filters = "Visual Script (*.dvs)";
        QString fileName = ui->GetOpenFileName(DAVA::mainWindowKey, fParams);
        if (fileName.isEmpty() == true)
        {
            return static_cast<VisualScriptAnotherScriptNode*>(nullptr);
        }

        String scriptPath = fileName.toStdString();
        { // save settings
            VisualScriptEditorDialogSettings* settings = accessor->GetGlobalContext()->GetData<VisualScriptEditorDialogSettings>();
            DVASSERT(settings != nullptr);

            auto it = std::find(settings->recentScripts.begin(), settings->recentScripts.end(), scriptPath);
            if (it == settings->recentScripts.end())
            {
                settings->recentScripts.insert(settings->recentScripts.begin(), scriptPath);
                if (settings->recentScripts.size() > 10)
                {
                    settings->recentScripts.pop_back();
                }
            }
        }

        VisualScriptAnotherScriptNode* node = script->CreateNode<VisualScriptAnotherScriptNode>();
        node->SetScriptFilepath(scriptPath);
        return node;
    };
}

void VisualScriptRegistryModel::RegisterReflectedData()
{
    std::function<void(const Reflection& model)> parceReflection = [this, &parceReflection](const Reflection& model)
    {
        if (model.HasFields())
        {
            Vector<Reflection::Field> fields = model.GetFields();
            for (Reflection::Field& f : fields)
            {
                if (f.ref.HasFields())
                {
                    parceReflection(f.ref);
                }
                else
                {
                    FastName key = f.key.Get<FastName>();
                    Reflection ref = model;

                    String title = key.c_str();
                    title[0] = std::toupper(title[0], std::locale()); // Capitalize first letter

                    {
                        RegistryItemDescriptorData* descrData = CreateDynamicItem("Values", QString::fromLatin1(("Set" + title).c_str()));
                        descrData->creator = [ref, key, this]() {
                            const ReflectedType* scriptNodeType = ReflectedTypeDB::GetByPermanentName("VisualScriptSetVarNode");
                            return script->CreateNode(scriptNodeType, ref, key);
                        };
                    }

                    {
                        RegistryItemDescriptorData* descrData = CreateDynamicItem("Values", QString::fromLatin1(("Get" + title).c_str()));
                        descrData->creator = [ref, key, this]() {
                            const ReflectedType* scriptNodeType = ReflectedTypeDB::GetByPermanentName("VisualScriptGetVarNode");
                            return script->CreateNode(scriptNodeType, ref, key);
                        };
                    }
                }
            }
        }
    };

    for (const Reflection& model : dataContainer)
    {
        parceReflection(model);
    }
}

void VisualScriptRegistryModel::RegisterSubscripts()
{
    VisualScriptEditorDialogSettings* settings = accessor->GetGlobalContext()->GetData<VisualScriptEditorDialogSettings>();
    for (const String& pathStr : settings->recentScripts)
    {
        FilePath filepath = pathStr;

        RegistryItemDescriptorData* descrData = CreateDynamicItem("Another Script: Recent Scripts", filepath.GetBasename().c_str());
        descrData->creator = [this, filepath]()
        {
            VisualScriptAnotherScriptNode* node = script->CreateNode<VisualScriptAnotherScriptNode>();
            node->SetScriptFilepath(filepath);
            return node;
        };
    }
}

void VisualScriptRegistryModel::ReleaseReflectedItems(std::map<QString, std::vector<QtNodes::RegistryItemDescriptor*>>& reflectedItems)
{
    for (const std::pair<QString, std::vector<QtNodes::RegistryItemDescriptor*>>& pair : reflectedItems)
    {
        for (const QtNodes::RegistryItemDescriptor* descriptor : pair.second)
        {
            RegistryItemDescriptorData* descrData = static_cast<RegistryItemDescriptorData*>(descriptor->userData);
            delete descrData;
            delete descriptor;
        }
    }
    reflectedItems.clear();
}

void VisualScriptRegistryModel::BuildStaticModel()
{
    _registeredModelsCategory.clear();
    _categories.clear();
    _registeredModels.clear();
    _registeredTypeConverters.clear();

    ReleaseReflectedItems(staticReflectedItems);
    ReleaseReflectedItems(dynamicReflectedItems);
    RegisterEvents();
    RegisterControllers();
    RegisterReflectedMethods();
    RegisterSubscriptsAction();
}

std::unique_ptr<QtNodes::NodeDataModel> VisualScriptRegistryModel::Create(const QtNodes::RegistryItemDescriptor* descriptor)
{
    DVASSERT(descriptor != nullptr);
    RegistryItemDescriptorData* descrData = static_cast<RegistryItemDescriptorData*>(descriptor->userData);
    DVASSERT(descrData != nullptr);

    VisualScriptNode* scriptNode = descrData->creator();
    if (scriptNode != nullptr)
    {
        return std::make_unique<VisualScriptNodeModel>(accessor, ui, script, scriptNode);
    }

    return std::unique_ptr<VisualScriptNodeModel>();
}

} //DAVA
