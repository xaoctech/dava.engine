#include "EditorPhysics/Private/EditorIntegrationHelper.h"

#include <TArc/DataProcessing/DataContext.h>
#include <TArc/DataProcessing/DataNode.h>

#include <Scene3D/Scene.h>
#include <Reflection/ReflectedTypeDB.h>

namespace EditorPhysicsDetail
{
DAVA::Scene* ExtractScene(DAVA::TArc::DataContext* context)
{
    const DAVA::ReflectedType* sceneDataType = DAVA::ReflectedTypeDB::GetByPermanentName("SceneData");
    if (sceneDataType == nullptr)
    {
        return nullptr;
    }

    DAVA::TArc::DataNode* data = context->GetData(sceneDataType);
    DAVA::Reflection ref = DAVA::Reflection::Create(DAVA::ReflectedObject(data));
    if (ref.IsValid() == false)
    {
        return nullptr;
    }

    DAVA::Reflection sceneField = ref.GetField(DAVA::FastName("ScenePtr"));
    if (sceneField.IsValid() == false)
    {
        return nullptr;
    }

    return sceneField.GetValue().Cast<DAVA::Scene*>();
}
} // namespace EditorPhysics
