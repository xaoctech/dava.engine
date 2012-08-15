#include "PointerHolder.h"

#include "DAVAEngine.h"
using namespace DAVA;

void RegisterBasePointerTypes()
{
    RegisterPointerType<void *>(String("void *"));
    RegisterPointerType<int32 *>(String("int32 *"));
    RegisterPointerType<float32 *>(String("float32 *"));
}
