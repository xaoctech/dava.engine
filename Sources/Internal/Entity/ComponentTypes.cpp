#include "Entity/ComponentType.h"
#include "FileSystem/Logger.h"

namespace DAVA 
{
uint64 ComponentType::globalBit = 1;
uint64 ComponentType::globalIndex = 0;
    

void ComponentType::Init()
{
    bit = globalBit;
    index = globalIndex;
    globalBit <<= 1;
    globalIndex++;
    if (globalIndex >= 64)
    {
        Logger::Error("Need to think about larger ComponentType");
    }
}





}