#include "ModuleManager/ModuleManager.h"
#include "ModuleManager/IModule.h"

#if defined(__DAVAENGINE_ANDROID__)
// use it hack for now because  NDK build not support generating
// ModuleManager like CMake
#include "../../Modules/Spine/Sources/SpineModule.h"
#endif

namespace DAVA
{
Vector<IModule*> CreateModuleInstances(Engine* engine)
{
    return Vector<IModule*>();
    Vector<IModule*> modules;
#if defined(__DAVAENGINE_ANDROID__)
    modules.emplace_back(new SpineModule(engine));
#endif
    return modules;
}
}
