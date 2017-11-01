#include "UI/Flow/Services/DataUIService.h"
#include "Engine/Engine.h"
#include "Engine/EngineContext.h"
#include "Logger/Logger.h"
#include "Reflection/ReflectionRegistrator.h"
#include "UI/Flow/UIFlowContext.h"
#include "UI/UIControlSystem.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(DataUIService)
{
    ReflectionRegistrator<DataUIService>::Begin()
    .ConstructorByPointer()
    .DestructorByPointer([](DataUIService* s) { delete s; })
    .Method("SetDataDirty", &DataUIService::SetDataDirty)
    .End();
}

void DataUIService::SetDataDirty(const Reflection& ref)
{
    // TODO: Uncomment after merging Bindings
    //    UIDataBindingSystem* dbs = Engine::Instance()->GetContext()->uiControlSystem->GetSystem<UIDataBindingSystem>();
    //    if (dbs)
    //    {
    //        dbs->SetDataDirty(ref.GetValueObject().GetVoidPtr());
    //    }
}
}
