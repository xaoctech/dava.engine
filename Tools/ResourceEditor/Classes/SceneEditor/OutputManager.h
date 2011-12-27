#ifndef __OUTPUT_MANAGER_H__
#define __OUTPUT_MANAGER_H__

#include "DAVAEngine.h"

using namespace DAVA;

class OutputControl;
class OutputManager: public Singleton<OutputManager>
{
    
public:
    OutputManager();

    void SetActiveOutput(OutputControl *output);
    
    void Log(const WideString &message);
    void Warning(const WideString &message);
    void Error(const WideString &message);

    
protected:
    
    OutputControl *outputControl;
};



#endif // __OUTPUT_MANAGER_H__