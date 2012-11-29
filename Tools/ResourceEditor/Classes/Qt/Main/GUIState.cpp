#include "GUIState.h"

using namespace DAVA;

GUIState::GUIState()
{
    SetNeedUpdatedFileMenu(true);
    SetNeedUpdatedToolsMenu(true);
    SetNeedUpdatedToolbar(true);
    SetNeedUpdatedStatusbar(true);
}

GUIState::~GUIState()
{
}


