#include "OutputManager.h"

#include "OutputControl.h"

OutputManager::OutputManager()
{
    outputControl = NULL;
}

void OutputManager::SetActiveOutput(OutputControl *output)
{
    outputControl = output;
}

void OutputManager::Log(const WideString &message)
{
    if(outputControl)
    {
        outputControl->Log(message);
    }
}

void OutputManager::Warning(const WideString &message)
{
    if(outputControl)
    {
        outputControl->Warning(message);
    }
}

void OutputManager::Error(const WideString &message)
{
    if(outputControl)
    {
        outputControl->Error(message);
    }
}


