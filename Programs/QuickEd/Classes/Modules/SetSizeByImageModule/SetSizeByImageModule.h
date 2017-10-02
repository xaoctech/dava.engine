#pragma once

#include <TArc/Core/ClientModule.h>
#include <TArc/Utils/QtConnections.h>

#include <Reflection/Reflection.h>

class SetSizeByImageModule : public DAVA::TArc::ClientModule
{
    // ClientModule
    void PostInit() override;

    void OnSetSizeFromImage();

    DAVA::TArc::QtConnections connections;
    DAVA_VIRTUAL_REFLECTION(SetSizeByImageModule, DAVA::TArc::ClientModule);
};
