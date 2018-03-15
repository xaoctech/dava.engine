#pragma once

#include <TArc/DataProcessing/TArcDataNode.h>

#include <Base/Vector.h>
#include <FileSystem/FilePath.h>
#include <Reflection/Reflection.h>
#include <Reflection/ReflectionRegistrator.h>

namespace QtNodes
{
class FlowView;
}

namespace DAVA
{
class VisualScript;
class VisualScriptFlowScene;
class VisualScriptFlowView;

class VisualScriptEditorReflectionHolder final
{
public:
    //trick for property panel
    Vector<Reflection> reflectedModels;
};

class ScriptDescriptor final
{
public:
    VisualScript* script = nullptr;
    bool isScriptOwner = false;

    VisualScriptFlowScene* flowScene = nullptr;
    VisualScriptFlowView* flowView = nullptr;
    VisualScriptEditorReflectionHolder* reflectionHolder = nullptr;

    FilePath scriptPath;
};

class VisualScriptEditorData : public TArcDataNode
{
public:
    static const char* scriptDescriptorFieldName;
    static const char* scriptNodesFieldName;
    static const char* reflectionHolderProperty;

    ScriptDescriptor* activeDescriptor = nullptr;

    Reflection GetReflection() const
    {
        if (activeDescriptor != nullptr)
        {
            return Reflection::Create(ReflectedObject(activeDescriptor->reflectionHolder));
        }

        static VisualScriptEditorReflectionHolder reflectionHolder;
        return Reflection::Create(ReflectedObject(&reflectionHolder));
    }

    DAVA_VIRTUAL_REFLECTION(VisualScriptEditorData, TArcDataNode);
};

//template <>
//bool AnyCompare<VisualScriptEditorReflectionHolder>::IsEqual(const Any& v1, const Any& v2);
//extern template struct AnyCompare<VisualScriptEditorReflectionHolder>;

} //DAVA
