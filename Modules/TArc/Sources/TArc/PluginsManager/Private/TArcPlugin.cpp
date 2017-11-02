#include "TArc/PluginsManager/TArcPlugin.h"

#include <Engine/EngineContext.h>
#include <Reflection/ReflectedTypeDB.h>
#include <Base/Type.h>
#include <Base/FastName.h>

namespace DAVA
{
TArcPlugin::TArcPlugin(const EngineContext* context)
{
    TypeDB::GetLocalDB()->SetMasterDB(context->typeDB);
    FastNameDB::GetLocalDB()->SetMasterDB(context->fastNameDB);
    ReflectedTypeDB::GetLocalDB()->SetMasterDB(context->reflectedTypeDB);
}
} // namespace DAVA